#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "ngx_func.h"     
#include "ngx_c_conf.h"   

//静态成员赋值
CConfig *CConfig::m_instance = NULL;
std::mutex CConfig::m_mutex;
//构造函数
CConfig::CConfig()
{		
}
//析构函数
CConfig::~CConfig()
{    
	std::vector<LPCConfItem>::iterator pos;	
	for(pos = m_ConfigItemList.begin(); pos != m_ConfigItemList.end(); ++pos)
	{		
		delete (*pos);
	}
	m_ConfigItemList.clear(); 
    return;
}

//装载配置文件
bool CConfig::Load(const char *pconfName) 
{   
    auto trimStr = [](char* s) {
        std::string str(s);
        // 去左空格
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char c) { return !std::isspace(c); }));
        // 去右空格
        str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), str.end());
        strcpy(s, str.c_str());
        };

    FILE *fp;
    fp = fopen(pconfName,"r");
    if(fp == NULL)
        return false;

    char  linebuf[501];   
    
    while(!feof(fp)) 
    {    
        if(fgets(linebuf,500,fp) == NULL) 
            continue;

        if(linebuf[0] == 0)
            continue;

        if(*linebuf==';' || *linebuf==' ' || *linebuf=='#' || *linebuf=='\t'|| *linebuf=='\n')
			continue;
        
    lblprocstring:
		if(strlen(linebuf) > 0)
		{
			if(linebuf[strlen(linebuf)-1] == 10 || linebuf[strlen(linebuf)-1] == 13 || linebuf[strlen(linebuf)-1] == 32) 
			{
				linebuf[strlen(linebuf)-1] = 0;
				goto lblprocstring;
			}		
		}
        if(linebuf[0] == 0)
            continue;
        if(*linebuf=='[') 
			continue;

        char *ptmp = strchr(linebuf,'=');
        if(ptmp != NULL)
        {
            LPCConfItem p_confitem = new CConfItem;                    
            memset(p_confitem,0,sizeof(CConfItem));
            strncpy(p_confitem->ItemName,linebuf,(int)(ptmp-linebuf)); 
            strcpy(p_confitem->ItemContent,ptmp+1);                    

            trimStr(p_confitem->ItemName);
            trimStr(p_confitem->ItemContent);

            m_ConfigItemList.push_back(p_confitem);  //内存要释放，因为这里是new出来的 
        } 
    } 

    fclose(fp); 
    return true;
}

const char *CConfig::GetString(const char *p_itemname)
{
	std::vector<LPCConfItem>::iterator pos;	
	for(pos = m_ConfigItemList.begin(); pos != m_ConfigItemList.end(); ++pos)
	{	
		if(strcasecmp( (*pos)->ItemName,p_itemname) == 0)
			return (*pos)->ItemContent;
	}
	return NULL;
}
int CConfig::GetIntDefault(const char *p_itemname,const int def)
{
	std::vector<LPCConfItem>::iterator pos;	
	for(pos = m_ConfigItemList.begin(); pos !=m_ConfigItemList.end(); ++pos)
	{	
		if(strcasecmp( (*pos)->ItemName,p_itemname) == 0)
			return atoi((*pos)->ItemContent);
	}
	return def;
}



