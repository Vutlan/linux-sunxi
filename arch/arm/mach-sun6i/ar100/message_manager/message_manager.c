/*
 *  arch/arm/mach-sun6i/ar100/message_manager/message_manager.c
 *
 * Copyright (c) 2012 Allwinner.
 * sunny (sunny@allwinnertech.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "message_manager_i.h"

/*
 * the queue of freed message, 
 * mainly for allocate message performance.
 */
static struct ar100_message_queue free_list;

/* the start and end of message pool */
static struct ar100_message *message_start;
static struct ar100_message *message_end;

/* spinlock for this module */
static spinlock_t    msg_mgr_lock;
static unsigned long msg_mgr_flag;

#define AR100_SEM_CACHE_MAX	(8)

struct ar100_semaphore_allocator
{
	unsigned int	  sem_number;
	struct semaphore *sem_cache[AR100_SEM_CACHE_MAX];
};

static struct ar100_semaphore_allocator sem_allocator;

/**
 * initialize message manager.
 * @para:  none.
 *
 * returns:  0 if initialize succeeded, others if failed.
 */
int ar100_message_manager_init(void)
{
	/* initialize message pool start and end */
	message_start = (struct ar100_message *)(ar100_sram_a2_vbase + AR100_MESSAGE_POOL_START);
	message_end   = (struct ar100_message *)(ar100_sram_a2_vbase + AR100_MESSAGE_POOL_END);
	
	/* initialize free list */
	free_list.head   = NULL;
	free_list.tail   = NULL;
	free_list.number = 0;
	
	/* initialzie semaphore allocator */
	sem_allocator.sem_number = 0;
	
	/* initialize message manager spinlock */
	spin_lock_init(&(msg_mgr_lock));
	msg_mgr_flag = 0;
	
	return 0;
}

/**
 * exit message manager.
 * @para:  none.
 *
 * returns:  0 if exit succeeded, others if failed.
 */
int ar100_message_manager_exit(void)
{
	return 0;
}

static struct semaphore *ar100_semaphore_allocate(void)
{
	struct semaphore *sem;
	
	if (sem_allocator.sem_number) {
		/* allocate from cache */
		spin_lock_irqsave(&(msg_mgr_lock), msg_mgr_flag);
		sem_allocator.sem_number--;
		sem = sem_allocator.sem_cache[sem_allocator.sem_number];
		sem_allocator.sem_cache[sem_allocator.sem_number] = NULL;
		spin_unlock_irqrestore(&(msg_mgr_lock), msg_mgr_flag);
	} else {
		/* allocate from kmem */
		sem = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	}
	
	/* initialize allocated semaphore */
	sema_init(sem, 0);
	
	return sem;
}

static int ar100_semaphore_free(struct semaphore *sem)
{
	if (sem_allocator.sem_number < AR100_SEM_CACHE_MAX) {
		/* free to cache */
		spin_lock_irqsave(&(msg_mgr_lock), msg_mgr_flag);
		sem_allocator.sem_cache[sem_allocator.sem_number] = sem;
		sem_allocator.sem_number++;
		spin_unlock_irqrestore(&(msg_mgr_lock), msg_mgr_flag);
	} else {
		/* free to kmem */
		kfree(sem);
	}
	
	return 0;
}

/**
 * allocate one message frame. mainly use for send message by message-box,
 * the message frame allocate form messages pool shared memory area.
 * @para:  none.
 *
 * returns:  the pointer of allocated message frame, NULL if failed;
 */
struct ar100_message *ar100_message_allocate(unsigned int msg_attr)
{
	struct ar100_message *pmessage = NULL;
	struct ar100_message *palloc   = NULL;
	
	/* first find in free_list */
	if (free_list.number) {
		/*
		 * free_list have cached message,
		 * allocate the head node.
		 */
		AR100_INF("free_list.head   = 0x%x.\n", (unsigned int)free_list.head);
		AR100_INF("free_list.number = %d\n", free_list.number);
		spin_lock_irqsave(&(msg_mgr_lock), msg_mgr_flag);
		palloc = free_list.head;
		free_list.head = palloc->next;
		free_list.number--;
		palloc->next   = 0;
		palloc->attr   = msg_attr;
		spin_unlock_irqrestore(&(msg_mgr_lock), msg_mgr_flag);
		if (msg_attr & AR100_MESSAGE_ATTR_SOFTSYN) {
			/* syn message,allocate one semaphore for private */
			palloc->private = ar100_semaphore_allocate();
		} else {
			palloc->private = NULL;
		}
		AR100_INF("message allocate from free_list\n");
		return palloc;
	}
	
	/*
	 * cached free_list finded fail, 
	 * use spinlock 0 to exclusive with ac327.
	 */
	ar100_hwspin_lock_timeout(0, AR100_SPINLOCK_TIMEOUT);
	
	/*
	 * seach from the start of message pool every time.
	 * maybe have other more good choice.
	 * by sunny at 2012-5-13 10:36:50.
	 */
	pmessage = message_start;
	while (pmessage < message_end) {
		if (pmessage->state == AR100_MESSAGE_FREED) {
			/* find free message in message pool, allocate it */
			palloc = pmessage;
			palloc->state  = AR100_MESSAGE_ALLOCATED;
			palloc->next   = 0;
			palloc->attr   = msg_attr;
			AR100_INF("message allocate from message pool\n");
			AR100_INF("ar100_message_allocate:palloc:0x%x\n", (unsigned int)palloc);
			break;
		}
		/* next message frame */
		pmessage++;
	}
	
	/* unlock hwspinlock 0 */
	ar100_hwspin_unlock(0);
	
	if (palloc == NULL) {
		AR100_ERR("allocate message frame fail\n");
	}
	
	if (msg_attr & AR100_MESSAGE_ATTR_SOFTSYN) {
		/* syn message,allocate one semaphore for private */
		palloc->private = ar100_semaphore_allocate();
	} else {
		palloc->private = NULL;
	}
	
	return palloc;
}

/**
 * free one message frame. mainly use for process message finished, 
 * free it to messages pool or add to free message queue.
 * @pmessage:  the pointer of free message frame.
 *
 * returns:  none.
 */
void ar100_message_free(struct ar100_message *pmessage)
{
	/* try to add free_list first */
	if (free_list.number < AR100_MESSAGE_CACHED_MAX) {
		if (pmessage->attr & AR100_MESSAGE_ATTR_SOFTSYN) {
			/* free message semaphore first */
			ar100_semaphore_free((struct semaphore *)(pmessage->private));
			pmessage->private = NULL;
		}
		
		AR100_INF("insert message [%x] to free_list\n", (unsigned int)pmessage);
		AR100_INF("free_list number : %d\n", free_list.number);
		
		/* cached this message, message state: ALLOCATED */
		spin_lock_irqsave(&(msg_mgr_lock), msg_mgr_flag);
		if (free_list.number) {
			/* add to the tail of free_list */
			free_list.tail->next = pmessage;
		} else {
			/* add to head of free_list */
			free_list.head = pmessage;
		}
		free_list.tail = pmessage;
		free_list.number++;
		pmessage->next  = NULL;
		pmessage->state = AR100_MESSAGE_ALLOCATED;
		spin_unlock_irqrestore(&(msg_mgr_lock), msg_mgr_flag);
	} else {
		if (pmessage->attr & AR100_MESSAGE_ATTR_SOFTSYN) {
			/* free message semaphore first */
			ar100_semaphore_free((struct semaphore *)(pmessage->private));
			pmessage->private = NULL;
		}
		/*
		 * free to message pool,
		 * set message state as FREED.
		 */
		ar100_hwspin_lock_timeout(0, AR100_SPINLOCK_TIMEOUT);
		pmessage->state = AR100_MESSAGE_FREED;
		pmessage->next  = NULL;
		ar100_hwspin_unlock(0);
	}
}

/**
 * notify system that one message coming.
 * @pmessage:  the pointer of coming message frame.
 *
 * returns:  0 if notify succeeded, other if failed.
 */
int ar100_message_coming_notify(struct ar100_message *pmessage)
{
	int   ret;
	
	/* ac327 receive message to ar100 */
	AR100_LOG("-------------------------------------------------------------\n");
	AR100_LOG("                MESSAGE FROM AR100                           \n");
	AR100_LOG("message addr : %x\n", (u32)pmessage);
	AR100_LOG("message type : %x\n", pmessage->type);
	AR100_LOG("message attr : %x\n", pmessage->attr);
	AR100_LOG("-------------------------------------------------------------\n");
	
	/* message per-process */
	pmessage->state = AR100_MESSAGE_PROCESSING;
	
	/* process message */
	switch (pmessage->type) {
		case AR100_AXP_INT_COMING_NOTIFY: {
			AR100_INF("pmu interrupt coming notify\n");
			ret = ar100_axp_int_notify(pmessage);
			pmessage->result = ret;
			break;
		}
		default : {
			AR100_ERR("invalid message type for ac327 process\n");
			ret = -EINVAL;
			break;
		}
	}
	/* message post process */
	pmessage->state = AR100_MESSAGE_PROCESSED;
	if ((pmessage->attr & AR100_MESSAGE_ATTR_SOFTSYN) || 
		(pmessage->attr & AR100_MESSAGE_ATTR_HARDSYN)) {
		/* synchronous message, should feedback process result */
		ar100_hwmsgbox_feedback_message(pmessage, AR100_SEND_MSG_TIMEOUT);
	} else {
		/*
		 * asyn message, no need feedback message result,
		 * free message directly.
		 */
		ar100_message_free(pmessage);	
	}
	
	return ret;
}
