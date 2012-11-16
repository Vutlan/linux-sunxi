/*
 *  gsensor.c - Linux kernel modules for  Detection Sensor 
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/hwmon.h>
#include <linux/input-polldev.h>
#include <linux/device.h>
#include <linux/earlysuspend.h>

#include <mach/system.h>
#include <mach/hardware.h>
#include <mach/sys_config.h>

#include<linux/types.h>
#include<linux/fs.h>
#include<linux/string.h>
#include<asm/uaccess.h>

#define NOTE_INFO1 ";Behind the equals sign said detected equipment corresponding to the name of the driver\n"
#define NOTE_INFO2 ";Note: don't change the file format!\n"
#define GSENSOR_DEVICE_KEY_NAME  "gsensor_module_name"
#define CTP_DEVICE_KEY_NAME      "ctp_module_name"
#define FILE_DIR  "system/usr/device.info"

#define STRING_LENGTH           (128)
#define FILE_LENGTH             (1024)
#define NAME_LENGTH             (32)
#define GSENSOR_SUPPORT_NUMBER  (9)
#define CTP_SUPPORT_NUMBER      (5)
#define ADDRESS_NUMBER          (3)
#define REG_VALUE_NUMBER        (4)
#define DEFAULT_TOTAL_ROW       (4)

static char gsensor_name[NAME_LENGTH] = {'\0'};
static char ctp_name[NAME_LENGTH] = {'\0'};
static struct i2c_client *temp_client;


static struct file *filp = NULL;
static int g_device_used = 0;
static int c_device_used = 0;
static __u32 gsensor_twi_id = 0;
static __u32 ctp_twi_id = 0;
static int write = 0;
static int total_raw = DEFAULT_TOTAL_ROW;

enum twi_device_type{
        TWI_GSENSOR = 0,
        TWI_CTP,
};
 
struct id{
        int gsensor_id;
        int ctp_id;   
}write_id = {2,3};
    
struct device_config_info{
        char str_info[STRING_LENGTH];
        int str_id;
};

static struct device_config_info config_info[STRING_LENGTH];

struct base_info{
        char name[NAME_LENGTH];
        unsigned short i2c_address[ADDRESS_NUMBER];
        unsigned short chip_id_reg;
        unsigned short chip_id_reg_value[REG_VALUE_NUMBER];
};
static struct base_info sensors[GSENSOR_SUPPORT_NUMBER] = {
        {"bma250",{0x18,0x08,0x38},0x00,{0x02,0x03}},
        {"mma7660",{0x4c},0x00,{0x00}},
        {"dmard06",{0x1c},0x0f,{0x06}},
        {"mma8452",{0x1c,0x1d},0x0d,{0x2A}},
        {"kxtik",{0x0f},0x0f,{0x05,0x08}},
        {"mxc622x",{0x15},0x00,{0x00}},
        {"afa750",{0x3d},0x37,{0x3d,0x3c}},
        {"mma865x",{0x1d},0x0d,{0x4A,0x5A}},
        {"lis35de",{0x1c,0x1d},0x00,{0x00}},
        };

static struct base_info ctps[CTP_SUPPORT_NUMBER] = {
        {"ft5x_ts",{0x38},0xa3,{0x55,0x08,0x02,0x06}},
        {"gt82x",{0x5d},0xf7d,{0x13,0x27,0x28}},
        {"gt818_ts",{0x5d},0x716,{0x18}},
        {"gt811_ts",{0x5d},0x715,{0x11}},
        {"zet6221",{0x76},0x00,{0x00}},
        };



/*Function as i2c_master_send, and return 1 if operation is successful.*/ 
int i2c_write_bytes(struct i2c_client *client, uint8_t *data, uint16_t len)
{
	struct i2c_msg msg;
	int ret=-1;
	
	msg.flags = !I2C_M_RD;//写消息
	msg.addr = client->addr;
	msg.len = len;
	msg.buf = data;		
	
	ret=i2c_transfer(client->adapter, &msg,1);
	return ret;
}
EXPORT_SYMBOL(i2c_write_bytes);
/*Function as i2c_master_receive, and return 2 if operation is successful.*/
static int i2c_read_bytes_addr16(struct i2c_client *client, uint8_t *buf, uint16_t len)
{
	struct i2c_msg msgs[2];
	int ret=-1;
	//发送写地址
	msgs[0].flags = !I2C_M_RD;
	msgs[0].addr = client->addr;
	msgs[0].len = 2;		//data address
	msgs[0].buf = buf;
	//接收数据
	msgs[1].flags = I2C_M_RD;//读消息
	msgs[1].addr = client->addr;
	msgs[1].len = len-2;
	msgs[1].buf = buf+2;
	
	ret=i2c_transfer(client->adapter, msgs, 2);
	return ret;
}
EXPORT_SYMBOL(i2c_read_bytes_addr16);
/*
*********************************************************************************************************
*                                   i2c_test
*
*Description: By writing device address, testing equipment, whether or not successful communication
*
*Arguments  :client      Contain equipment address of the i2c_client structure
*
*Return     : result;
*               = true,      Communication success;
*               = false,     Communication failed!
*********************************************************************************************************
*/
bool i2c_test(struct i2c_client * client)
{
        int ret,retry;
        uint8_t test_data[1] = { 0 };	//only write a data address.
        
        for(retry=0; retry < 2; retry++)
        {
                ret =i2c_write_bytes(client, test_data, 1);	//Test i2c.
        	if (ret == 1)
        	        break;
        	msleep(50);
        }
        
        return ret==1 ? true : false;
} 
EXPORT_SYMBOL(i2c_test);

/*
*********************************************************************************************************
*                                   i2c_get_device_number
*
*Description: Through the device name for equipment storage locations
*
*Arguments  :info                   Storage equipment information structure
*            support_number         Storage equipment information structure body size(support equipment number)
*            name                   To find the device name
*Return     : result;
*               = suppotr_number,   Equipment in the position of the array;
*               = -1,               In the array device name not found!
*********************************************************************************************************
*/
static int i2c_get_device_number(struct base_info *info,int support_number,char *name)
{
        int ret = -1;
        if(strlen(name)){
                while(support_number--){
                        if (!strncmp(name, info[support_number].name, strlen(info[support_number].name))){
                                printk("number: %d \n",support_number);
                                return support_number;
                        }
                }
        }
        printk("-----the name is null !-----\n");
        return ret;
}
/*
*********************************************************************************************************
*                                   i2c_get_unuse_name
*
*Description: Will not need testing equipment of i2c address zero
*
*Arguments  :info                   Storage equipment information structure
*            support_number         Storage equipment information structure body size(support equipment number)
*            main_name              sys_config1.fex Device list the main key name
*********************************************************************************************************
*/
static void i2c_get_use_list(struct base_info *info,int support_number,char * main_name)
{
        int i = 0;
        int list_value = -1;
        int ret = -1;
        while(support_number--){
                i = 0;
                if(SCRIPT_PARSER_OK != (ret = script_parser_fetch(main_name, info[support_number].name, &list_value, 1))){
	                pr_err("%s: script_parser_fetch err.ret = %d. \n", __func__, ret);
	                continue;
	        }
                 while((info[support_number].i2c_address[i++]) && (i < 3)){
	                //pr_info("name:%s,value:%d\n",info[support_number].name,list_value);
	                if(list_value == 0){
	                        info[support_number].i2c_address[i-1] = 0x00;      
	                }    
                }
        } 
 }

/*
*********************************************************************************************************
*                                   gsensor_fetch_sysconfig_para
*
*Description: Read sys_config1. Fex the configuration information
*Return     : result;
*               = 0,  Read correct
*               = -1, Read error!
*********************************************************************************************************
*/
static int gsensor_fetch_sysconfig_para(void)
{
	int ret = -1;
	int gsensor_used = 0;
		
	printk("========%s===================\n", __func__);
	 
	if(SCRIPT_PARSER_OK != (ret = script_parser_fetch("gsensor_para", "gsensor_used", &gsensor_used, 1))){
	        pr_err("%s: script_parser_fetch err.ret = %d. \n", __func__, ret);
	        goto script_parser_fetch_err;
	}

	if(1 == gsensor_used ){               
        	if(SCRIPT_PARSER_OK != script_parser_fetch("gsensor_para", "gsensor_twi_id", &gsensor_twi_id, 1)){
        	        pr_err("%s: script_parser_fetch err. \n", __func__);
        		goto script_parser_fetch_err;
        	}
        	printk("%s: gsensor_twi_id is %d. \n", __func__, gsensor_twi_id);
        	
        	
        	if(SCRIPT_PARSER_OK != (ret = script_parser_fetch("gsensor_list_para", "gsensor_det_used", &g_device_used, 1))){
	                pr_err("%s: script_parser_fetch err.ret = %d. \n", __func__, ret);
	                goto script_parser_fetch_err;
	        } 
	        if(g_device_used)
                        i2c_get_use_list(sensors,GSENSOR_SUPPORT_NUMBER,"gsensor_list_para");
                ret = 0;
		
	}else{
	        pr_err("%s: gsensor_unused. \n",  __func__);
		ret = -1;
	}

	return ret;

script_parser_fetch_err:
        pr_notice("line:%d:=========script_parser_fetch_err============\n",__LINE__);
	return ret;

}
/*
*********************************************************************************************************
*                                   ctp_fetch_sysconfig_para
*
*Description: Read sys_config1. Fex the configuration information
*Return     : result;
*               = 0,  Read correct
*               = -1, Read error!
*********************************************************************************************************
*/
static int ctp_fetch_sysconfig_para(void)
{
        int ret = -1;
        int ctp_used = 0;
	        
        printk("========%s===================\n", __func__);
         
        if(SCRIPT_PARSER_OK != (ret = script_parser_fetch("ctp_para", "ctp_used", &ctp_used, 1))){
                pr_err("%s: ctp_used script_parser_fetch err.ret = %d. \n", __func__, ret);
                goto script_parser_fetch_err;
        }
 
        if(1 == ctp_used){
                if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_twi_id", &ctp_twi_id, 1)){
                	pr_err("%s: script_parser_fetch err. \n", __func__);
                	goto script_parser_fetch_err;
                }
                printk("%s: ctp_twi_id is %d. \n", __func__, ctp_twi_id);
                if(SCRIPT_PARSER_OK != (ret = script_parser_fetch("ctp_list_para", "ctp_det_used", &c_device_used, 1))){
	                pr_err("%s: ctp_det_used script_parser_fetch err.ret = %d. \n", __func__, ret);
	                goto script_parser_fetch_err;
	        }
	        if(c_device_used)
	                i2c_get_use_list(ctps,CTP_SUPPORT_NUMBER,"ctp_list_para");
                ret = 0;
        	
        }else{
        	pr_err("%s: ctp_unused. \n",  __func__);
        	ret = -1;
        }
        
        return ret;
        
script_parser_fetch_err:
        pr_notice("line:%d:=========script_parser_fetch_err============\n",__LINE__);
        return ret;

}
static int is_alpha(char chr)
{
        int ret = -1;
        ret = ((chr >= 'a') && (chr <= 'z') ) ? 0 : 1;
        return ret;
}
/*
*********************************************************************************************************
*                                   i2c_get_device_name
*
*Description: For the name of the equipment
*Arguments: buf     Source string
*           name    Used for storage device name of the variable name
*Return     : result;
*               = 0, string error!
*               = 1, Read correct!
*********************************************************************************************************
*/

static int  i2c_get_device_name(char *buf ,char * name)
{
        int s1 = 0,i = 0;
        int ret = -1;
        char ch = '\"';
        char tmp_name[64];
        char * str1;
        char * str2;
        
        memset(&tmp_name,0,sizeof(tmp_name));
        if(!strlen (buf)){
                pr_info("%s:the buf is null!\n",__func__);
                return 0; 
        }
        str1 = strchr(buf,ch);
        str2 = strrchr(buf,ch);
        if((str1 == NULL) || (str2 == NULL)) {
                pr_info("the name is null!\n");
                return 1;
        }
                   
        ret = str1 - buf + 1;  
        s1 =  str2 - buf; 
        // pr_info("----ret : %d,s1 : %d---\n ",ret,s1);
        while(ret != s1)
        {
                tmp_name[i++] = buf[ret++];         
         
        }
        tmp_name[i] = '\0';
        strcpy(name,tmp_name);
        
        pr_info("name:%s\n",name);
        return 1;
}
/*
*********************************************************************************************************
*                                   i2c_match_device_keyname
*
*Description: To match the device key word
*Arguments: buf            Source string
*           key_name       Equipment key
*           name           Used for storage device name of the variable name
*Return     : result;
*               = 0, error!
*               = 1,correct!
*********************************************************************************************************
*/
static int i2c_match_device_keyname(char * src_buf,char * key_name,char * name)
{
        int ret = -1;
        
        if(!(strlen(src_buf) && strlen(key_name))){
                pr_info("%s:the src string is null!\n",__func__);
                return 0;
        }
        
        if(strncmp(src_buf,key_name,strlen(key_name))) {
               // pr_info("%s:Src_name and key_name don't match!\n",__func__);
                ret = 0;
        }else{
                if(i2c_get_device_name(src_buf,name))
                        ret = 1;
                else{ 
                        ret = 0;
                }
        }
        
        return ret;
              
}
/*
*********************************************************************************************************
*                                   i2c_match_device_name
*
*Description: According to the source file information find device name
*
*
*********************************************************************************************************
*/
static void i2c_match_device_name(void)
{
         int row_number = 0;
         
         row_number = total_raw;
	 while(row_number--){
	       // pr_info("config_info[%d].str_info:%s\n",row_number,config_info[row_number].str_info);
	    
	        if(is_alpha(config_info[row_number].str_info[0])){
	                continue;
	        }else if(i2c_match_device_keyname(config_info[row_number].str_info,GSENSOR_DEVICE_KEY_NAME,gsensor_name)){
	          
	                write_id.gsensor_id = config_info[row_number].str_id;
	                pr_info("gsensor_name:%s,id:%d\n",gsensor_name,write_id.gsensor_id);
	          
	        }else if(i2c_match_device_keyname(config_info[row_number].str_info,CTP_DEVICE_KEY_NAME,ctp_name)){
	        
    	                write_id.ctp_id = config_info[row_number].str_id;
    	                pr_info("ctp_name:%s,id:%d\n",ctp_name,write_id.ctp_id);
    	                             
	        }
	 }
}
/*
*********************************************************************************************************
*                                   i2c_analytic_config
*
*Description: Analysis of the configuration information, to break them down into a line of information 
*             stored in the device_config_info structure body in the variable
*Arguments: src_string     Configuration information source file
*           info           Storage line string structure body variable
*Return     : result;
*               = 1,correct!
*********************************************************************************************************
*/
static int i2c_analytic_config(char * src_string,struct device_config_info info[])
{
        int ret = -1;
        int i = 0,j = 0,k = 0;
        total_raw = 0;
        
        if(!strlen(src_string) ){
                pr_info("%s: the src string is null !\n",__func__);
                ret = 0;
                return ret;
        }         
        while(src_string[i++]){  
                info[k].str_info[j++] = src_string[i-1];
                if(src_string[i-1] == '\n'){
                        total_raw++; 
                        info[k].str_info[j] = '\0';
                        info[k].str_id = k;
//                        pr_info("info[%d].str_info:%s,total_raw:%d\n",k,info[k].str_info,total_raw);           
                        k += 1;
                        j = 0;
                    
                }   
        } 
        ret = 1;
        return ret;

}
/*
*********************************************************************************************************
*                                   i2c_get_twi_config
*
*Description: Reading configuration information
*             
*Arguments: tmp     Store configuration information variable
*Return     : result;
*             = ret,Read results!
*********************************************************************************************************
*/
static int i2c_get_twi_config(char * tmp)
{
        mm_segment_t old_fs;
        int ret;
        filp = filp_open(FILE_DIR,O_RDWR | O_CREAT,0666);
        
        if(IS_ERR(filp)){
                printk("open error ....\n");
                return -1;
        } 
        
        old_fs = get_fs();
        set_fs(get_ds());
        filp->f_op->llseek(filp,0,0);
        ret = filp->f_op->read(filp,tmp,FILE_LENGTH,&filp->f_pos);
        if(ret <= 0){
                pr_info("%s:read erro or read null !",__func__);
        }
        
        set_fs(old_fs);
        filp_close(filp,NULL);
        return ret;

}
/*
*********************************************************************************************************
*                                   i2c_update_config_info
*
*Description:Update the device name configuration information
*             
*Arguments: key_name       Equipment key
*           device_name    Equipment name
*           id             Equipment in configuration information of line number
*Return     : result;
*             = true, updated!
*             = false, Wrong information!
*********************************************************************************************************
*/
static bool i2c_update_config_info(char * key_name,char * device_name,int id)
{
        if((key_name == NULL)||(id < 0)){
                pr_info("%s:key_name is null or the id is error !\n",__func__);
                return false;
        }
        memset(&config_info[id].str_info,0,sizeof(config_info[id].str_info));
        sprintf(config_info[id].str_info,"%s=\"%s\"\n",key_name,device_name); 
        return true;
}

/*
*********************************************************************************************************
*                                   i2c_write_config
*
*Description:Will be updated configuration information to write configuration files
*             
*********************************************************************************************************
*/
static int i2c_write_config(void)
{
        mm_segment_t old_fs;
        int ret = 0,i =0;
        
        filp = filp_open(FILE_DIR,O_RDWR | O_CREAT | O_TRUNC,0666);
        if(IS_ERR(filp)){
                printk("open error ....\n");
                return -1;
        } 
        
        old_fs = get_fs();
        set_fs(get_ds());
        
        filp->f_op->llseek(filp,0,0);
        
        while(i < DEFAULT_TOTAL_ROW ){
                ret = filp->f_op->write(filp,config_info[i].str_info,strlen(config_info[i].str_info),&filp->f_pos);
                i++;
        }
        
        set_fs(old_fs);
        filp_close(filp,NULL);
//        pr_info("%s:ret = %d\n",__func__,ret);
        return ret;
}
/*
*********************************************************************************************************
*                                   i2c_write_device
*
*Description:Write equipment information
*             
*********************************************************************************************************
*/
static int i2c_write_device(void)
{
        int ret = 0;
        pr_info("%s:write device info !\n",__func__);
        if(write) {       
                memset(&config_info[0].str_info,0,sizeof(config_info[0].str_info));
                memset(&config_info[1].str_info,0,sizeof(config_info[1].str_info));
                strcpy(config_info[0].str_info,NOTE_INFO1);
                strcpy(config_info[1].str_info,NOTE_INFO2);
        }
                      
        i2c_write_config();
        
        return ret;
}
/*
*********************************************************************************************************
*                                   i2c_match_chip_id_value
*
*Description: The judge read chip id register value whether with equipment chip id the default values are equal
*Arguments: info      Device description information array
*           number    Matching value in the position of the array
*           value     To match the value
*Return     : result;
*               = 0,  Matching failure!
*               = 1, Matching success!
*********************************************************************************************************
*/
static bool i2c_match_chip_id_value(struct base_info *info,int number,int value)
{
        int i = 0;
        while(info[number].chip_id_reg_value[i])
        {
                if(info[number].chip_id_reg_value[i] == value){
                        printk("Chip id value equal!\n");
                        return true;
                }
                i++;
        }
        printk("Chip id value does not match!--value:%d--\n",value);
        return false;
}
/*
*********************************************************************************************************
*                                   i2c_device_i2c_test
*
*Description: Scanning device address, check the I2C communication success
*
*Arguments  :info                   Storage equipment information structure
*            i2c_address_number     The position of structures   
*            type                   Device type
*Return     : result;
*             = 1 ,Successful communication ;
*             = 0 ,Failure communication!
*********************************************************************************************************
*/
static int i2c_device_i2c_test(struct base_info *info,int i2c_address_number,enum twi_device_type type)
{
        int ret = 0,addr_scan = 0;
        __u32 twi_id = -1;
        
        struct i2c_adapter *adap ;

        switch(type){
        case TWI_CTP:
                twi_id = ctp_twi_id;
                break;
        case TWI_GSENSOR:
                twi_id = gsensor_twi_id;
                break;
        }
        
        adap = i2c_get_adapter(twi_id);
        temp_client->adapter = adap;	
        
        if (!i2c_check_functionality(temp_client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
                return -ENODEV;
          
        if(twi_id == temp_client->adapter->nr){
        
                while((info[i2c_address_number].i2c_address[addr_scan++]) && (addr_scan < 3)){
                        
                        temp_client->addr = info[i2c_address_number].i2c_address[addr_scan-1];
                        pr_info("%s:,name:%s ,addr: 0x%x\n",__func__,info[i2c_address_number].name,temp_client->addr); 
                        ret = i2c_test(temp_client);
                        if(!ret){
                        	continue;
                        }else{ 
                           // pr_info("%s: addr= 0x%x\n",__func__,temp_client->addr);          	    
                                pr_info("I2C connection sucess!\n");
                                break;
                        }  
                }   
        }
        return ret;
}
/*
*********************************************************************************************************
*                                   i2c_get_chip_id_value_addr16
*
*Description: By reading chip_id register for 16 bit address, get chip_id value
*
*Arguments  :address     Register address
*Return     : result;
*             Chip_id value
*********************************************************************************************************
*/
static int i2c_get_chip_id_value_addr16(unsigned short address)
{
        int value = -1;
        uint8_t buf[5] = {0};
        
        if(address & 0xff00){
                buf[0] = address >> 8;
                buf[1] = address & 0x00ff;
        }
        value = i2c_read_bytes_addr16(temp_client,buf,3);
        if(value != 2){
                pr_info("%s:read chip id error!",__func__);
        }
        value = buf[2] & 0xffff;
        //pr_info("buf[0]:0x%x,buf[1]:0x%x,buf[2]:0x%x,value:0x%x\n",buf[0],buf[1],buf[2],value);
        
        return value;     
}
/*
*********************************************************************************************************
*                                   i2c_get_chip_id_value_addr16
*
*Description: By reading chip_id register for 8 bit address, get chip_id value
*
*Arguments  :address     Register address
*Return     : result;
*             Chip_id value
*********************************************************************************************************
*/
static int i2c_get_chip_id_value_addr8(unsigned short address)
{
        int value = -1;
        
        value = i2c_smbus_read_byte_data(temp_client,address);
        value = value & 0xffff;

        return value;
}
/*
*********************************************************************************************************
*                                   i2c_chip_id_detect_gsensor
*
*Description:Gsensor matching the chip id value
*
*Arguments  :info                   Storage equipment information structure
*            i2c_address_number     The position of structures  
*Return     : result;
*             1 , success
*             0 , failure!
*********************************************************************************************************
*/

static int i2c_chip_id_detect_gsensor(struct base_info *info,int i2c_address_number)
{
        int detect_value = 0;
        int ret = -1,i = 0;
        unsigned short temp_addr;
        
        temp_addr = info[i2c_address_number].chip_id_reg;
         while (!((info[i2c_address_number].chip_id_reg_value[i++]) && (i < 3))){
                detect_value = 1;
                pr_info("-----%s:chip_id_reg value:0x%x",__func__,info[i2c_address_number].chip_id_reg_value[i-1]);
                return detect_value;
         }
        if(temp_addr & 0xff00){
                ret = i2c_get_chip_id_value_addr16(temp_addr);
        }else{
                ret = i2c_get_chip_id_value_addr8(temp_addr);
        }
        pr_info("-----%s:chip_id_reg:0x%x ,chip_id_value = 0x%x-----\n",__func__,temp_addr,ret);
        detect_value = i2c_match_chip_id_value(info,i2c_address_number,ret);

        
        return detect_value;                         
}
/*
*********************************************************************************************************
*                                   i2c_chip_id_detect_ctp
*
*Description:Gsensor matching the chip id value
*
*Arguments  :info                   Storage equipment information structure
*            i2c_address_number     The position of structures  
*Return     : result;
*             1 , success
*             0 , failure!
*********************************************************************************************************
*/
static int i2c_chip_id_detect_ctp(struct base_info *info,int i2c_address_number)
{
        int detect_value = 0;
        int ret = -1,i = 0;
        unsigned short temp_addr;
        
        temp_addr = info[i2c_address_number].chip_id_reg;
        
        while (!((info[i2c_address_number].chip_id_reg_value[i++]) && (i < 3))){
                detect_value = 1;
                pr_info("-----%s:chip_id_reg value:0x%x",__func__,info[i2c_address_number].chip_id_reg_value[i-1]);
                return detect_value;
        }
        if(temp_addr & 0xff00){
                ret = i2c_get_chip_id_value_addr16(temp_addr);
        }else{
                ret = i2c_get_chip_id_value_addr8(temp_addr);
        }
        pr_info("-----%s:chip_id_reg:0x%x ,chip_id_value = 0x%x-----\n",__func__,temp_addr,ret);
        detect_value = i2c_match_chip_id_value(info,i2c_address_number,ret);
        
        return detect_value;
}
/*
*********************************************************************************************************
*                                   i2c_update_device_name
*
*Description: The name of the update your equipment
*
*Arguments  :name      Equipment name
*            value     The i2c communication results, 0 indicates no find equipment, 1 said have found equipment
*            type      Device type
*********************************************************************************************************
*/
static void i2c_update_device_name(char * name,int value,enum twi_device_type type)
{
  
        switch(type){
        case TWI_GSENSOR: 
                memset(&gsensor_name,0,sizeof(gsensor_name));
                if(value){
                        strcpy(gsensor_name,name);
                }
                break;
        case TWI_CTP:
                memset(&ctp_name,0,sizeof(ctp_name));
                if(value){
                        strcpy(ctp_name,name);
                }
                break;
        }
         //pr_info("%s:name_update===gsensor_name:%s,ctp_name:%s",gsensor_name,ctp_name);
       
}
/*
*********************************************************************************************************
*                                   i2c_get_detect_value
*
*Description: Through the i2c address scanning, get support equipment
*Arguments: info              Device description information array
*           support_number    Equipment information describing the size of an array
*           address_number    Would like to start scanning position
*           type              Device type
*Return: result;
*        scan_number      Scanning frequency
*             
*********************************************************************************************************
*/
static int i2c_get_detect_value(struct base_info *info,int support_number,int address_number,enum twi_device_type type )
{
        int ret = 0;
        int scan_number = 0;
        int report_value = 0;
        int i2c_address_number = 0;
        
        i2c_address_number = address_number;
        
        while(scan_number < (support_number)){
              //  pr_info("scan_number:%d,i2c_address_number:%d\n",scan_number,i2c_address_number);
                scan_number++;
                i2c_address_number = (i2c_address_number == support_number) ? 0 : i2c_address_number;
                ret = i2c_device_i2c_test(info,i2c_address_number,type);
                if(!ret){
                        i2c_address_number++; 
        	         continue;
        	}   
        	 
        	switch(type) {
        	case TWI_GSENSOR://gsensor device
        	        report_value = i2c_chip_id_detect_gsensor(info,i2c_address_number);
        	        break;
        	case TWI_CTP://ctp device
                        report_value = i2c_chip_id_detect_ctp(info,i2c_address_number);
        	        break;
        	}
        	         
        	if(report_value){   	            
        	           break; 
        	}
                i2c_address_number++; 
         
        }
        i2c_update_device_name(info[i2c_address_number].name,report_value,type);
            
        return scan_number;
}

/*
*********************************************************************************************************
*                                   i2c_device_used_gsensor
*
*Description:Gsensor equipment test
*Return: result;
*        flag      Whether to need to update configuration information   
*********************************************************************************************************
*/
static int i2c_device_used_gsensor(void)
{
        int ret = -1,flag = 0;
        int gsensor_scan_number = -1;
            
        ret = i2c_get_device_number(sensors,GSENSOR_SUPPORT_NUMBER,gsensor_name);
        if(ret != -1){
                gsensor_scan_number = i2c_get_detect_value(sensors,GSENSOR_SUPPORT_NUMBER,ret,TWI_GSENSOR);
                flag |= ((gsensor_scan_number != 1) && (strlen(gsensor_name))) ?  0x02: 0;
        }
        else{
                i2c_get_detect_value(sensors,GSENSOR_SUPPORT_NUMBER,0,TWI_GSENSOR);
                flag |= strlen(gsensor_name) ? 0x02 : 0;
        }
        if(flag & 0x02){
                i2c_update_config_info(GSENSOR_DEVICE_KEY_NAME,gsensor_name,write_id.gsensor_id);
        }
            
        return flag;
}
/*
*********************************************************************************************************
*                                   i2c_device_used_ctp
*
*Description:ctp equipment test
*Return: result;
*        flag      Whether to need to update configuration information   
*********************************************************************************************************
*/
static int i2c_device_used_ctp(void)
{
        int ret = -1 ,flag = 0;
        int ctp_scan_number = -1;
       
        ret = i2c_get_device_number(ctps,CTP_SUPPORT_NUMBER,ctp_name);
        if(ret != -1){
                ctp_scan_number = i2c_get_detect_value(ctps,CTP_SUPPORT_NUMBER,ret,TWI_CTP);
                flag |= ((ctp_scan_number != 1) && (strlen(ctp_name))) ?  1 : 0;
        }else{
               i2c_get_detect_value(ctps,CTP_SUPPORT_NUMBER,0,TWI_CTP);
               flag |= strlen(ctp_name) ? 1 : 0;
        }
        if(flag & 0x01){
                i2c_update_config_info(CTP_DEVICE_KEY_NAME,ctp_name,write_id.ctp_id);
        }
        return flag;
}

static int i2c_hardware_detect(void)
{
        int flag = 0;
        struct i2c_client *client;
    
        client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
	if (!client)
                return -ENOMEM;

	temp_client = client;
	pr_info("g_device_used:%d,c_device_used:%d\n",g_device_used,c_device_used);
	
        if(c_device_used){
                msleep(200);
                flag |= i2c_device_used_ctp(); 
        }else{
                flag |= 1;
                memset(&ctp_name,0,sizeof(ctp_name));
                i2c_update_config_info(CTP_DEVICE_KEY_NAME,ctp_name,write_id.ctp_id);
        }
        if(g_device_used){
	        flag |= i2c_device_used_gsensor();
	}else{
                flag |= 1;
                memset(&gsensor_name,0,sizeof(gsensor_name));
                i2c_update_config_info(GSENSOR_DEVICE_KEY_NAME,gsensor_name,write_id.gsensor_id);
        }
       
        pr_info("write:%d,flag:%d",write,flag);
        if(write | flag){
                i2c_write_device();
        }
	kfree(temp_client);
	
	return 1;
    
}

static int __init i2c_hardware_init(void)
{
	int ret = -1,i = 0;
        static char tmp[FILE_LENGTH];
    
	printk("======%s=========. \n", __func__);
	
	write = 0;
	memset(&tmp,0,sizeof(tmp));
        memset(&gsensor_name,0,sizeof(gsensor_name));
        memset(&ctp_name,0,sizeof(ctp_name));
        
        while(i++ < STRING_LENGTH){
                memset(&config_info[i-1].str_info,0,sizeof(config_info[i-1].str_info));
       }

	if(gsensor_fetch_sysconfig_para()){
		printk("%s: gsensor_fetch_sysconfig_para err.\n", __func__);
	}
	if(ctp_fetch_sysconfig_para()){
		printk("%s: ctp_fetch_sysconfig_para err.\n", __func__);
	} 
	if(c_device_used | g_device_used) {
        	ret = i2c_get_twi_config(tmp);
        	if(ret <= 0){
        	          printk("get twi config erro!\n");
        	          write = 1;
        	}else{
        	        ret = i2c_analytic_config(tmp,config_info);
        	        if(!ret){
        	                printk("i2c_analytic_config erro!\n");
        	                write = 1;
        	        }else{
                                i2c_match_device_name();
                        }
                }
                if(!i2c_hardware_detect())
                        pr_info("the client is null!\n");
        }
	return 0;
}

static void __exit i2c_hardware_exit(void)
{
        printk("Gsensor Device driver exit!\n");    
    
}

MODULE_AUTHOR("Olina yin");
MODULE_DESCRIPTION("GSENSOR  Detection Sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.1");

module_init(i2c_hardware_init);
module_exit(i2c_hardware_exit);

