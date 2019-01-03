#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "main.h"

int main (int argc, char *argv[]){
    Road road;
    set_arg(argc, argv, &road);     
    print_title();
    int cod = get_all(&road);
    return cod;
    }

int print_title(){
    fprintf(stdout,"\"HIVE\",\"KEY\",\"cValues\",\"lpftLastWriteTime\",\"cbSecurityDescriptor\",\"DATA\",\"VALUE\"\n\n");
    return 0;
    }

int set_arg(int argc, char *argv[], Road *road){
    int narg = ( argc - 1 ) / 2 ;
    for(int n = 1;n<=narg;n++){
        if (strcmp(argv[(n*2)-1], "-h") == 0){
            set_hive(argv[(n*2)], road);
        }else if (strcmp(argv[(n*2)-1], "-k") == 0){
            strcpy(road->str,argv[(n*2)]);
        }
    }
    return 0;
    }

int set_hive(char *argv, Road *road){
    if (strcmp(argv, "hkcu") == 0){
        road->hive=HKEY_CURRENT_USER;
        strcpy(road->shive, "HKEY_CURRENT_USER");
    }
    else if (strcmp(argv, "hklm") == 0){
        road->hive=HKEY_LOCAL_MACHINE;
        strcpy(road->shive, "HKEY_LOCAL_MACHINE");
    }
    else if (strcmp(argv, "hkcr") == 0){
        road->hive=HKEY_CLASSES_ROOT;
        strcpy(road->shive, "HKEY_CLASSES_ROOT");
    }
    else if (strcmp(argv, "hkcc") == 0){
        road->hive=HKEY_CURRENT_CONFIG;
        strcpy(road->shive, "HKEY_CURRENT_CONFIG");
    }
    else if (strcmp(argv, "hku") == 0){
        road->hive=HKEY_USERS;
        strcpy(road->shive, "HKEY_USERS");
    }else {
        fprintf(stderr," %s hive invalid \n\n",argv );
        return 1;
    }
    return 0;
    }

int get_all(Road *road){
    KeyInfo keyinfo;
    int cod=query_key(road,&keyinfo);
    if(cod==0){
        if(keyinfo.cValues){
            get_values(road, &keyinfo);
        }
        if(keyinfo.cSubKeys){
            get_subKeys(road, &keyinfo );
        }
    }else if(cod!=0){
        //printf("Erreur query_key, retcod : %d \n",cod);
        //printf("  road->str : %s \n",road->str);
    }
    return cod;
    }

int query_key(Road *road, KeyInfo *k){
    DWORD retCode;
    if( ( retCode=RegOpenKeyEx( road->hive,road->str, 0, KEY_READ,&k->key))!=0){
        fprintf(stderr,"Erreur RegOpenKeyEx  \n");
        if(retCode==5){
            fprintf(stderr," -> Access is denied (%d) \n",(int)retCode);
            fprintf(stderr," %s  %s \n\n",road->shive,road->str);
        }
    }else{
    k->cchClassName=MAX_PATH;
    retCode = RegQueryInfoKey(
        k->key,                    // key handle
        k->achClass,                // buffer for class name
        &k->cchClassName,           // size of class string
        NULL,                    // reserved
        &k->cSubKeys,               // number of subkeys
        &k->cbMaxSubKey,            // longest subkey size
        &k->cchMaxClass,            // longest class string
        &k->cValues,            // number of values for this key
        &k->cchMaxValue,            // longest value name
        &k->cbMaxValueData,         // longest value data
        &k->cbSecurityDescriptor,   // security descriptor
        &k->ftLastWriteTime);
    }
    RegCloseKey(k->key);
    if(retCode){
        fprintf(stderr,"Erreur query_key  (%d) ",(int)retCode);
        if(retCode==ERROR_INVALID_HANDLE){
            fprintf(stderr," ERROR_INVALID_HANDLE  (%d) \n",(int)ERROR_INVALID_HANDLE);
            fprintf(stderr," %s \n\n",road->str);
        }
        if(retCode==ERROR_MORE_DATA){
            fprintf(stderr," ERROR_MORE_DATA (%d) \n",(int)ERROR_MORE_DATA);
            fprintf(stderr," %s \n\n",road->str);
        }else{
            fprintf(stderr," Erreur %d \n",(int)retCode);
            fprintf(stderr," %s \n\n",road->str);
        }
    }
    return (int)retCode;
    }

int get_values(Road *road, KeyInfo *keyinfo){
    DWORD retCode;
    EnumValue enumValue;

    LPSTR valueName[keyinfo->cchMaxValue];
    enumValue.lpValueName=valueName;

    LPBYTE valueData[keyinfo->cbMaxValueData];
    enumValue.lpData=valueData;

    flt_to_str(&keyinfo->ftLastWriteTime, road->lwt);
    make_richkey(keyinfo, road);
    enumValue.key=keyinfo->key;
    RegOpenKeyEx( road->hive,road->str, 0, KEY_READ,&enumValue.key);

    for (enumValue.dwIndex=0, retCode=ERROR_SUCCESS; enumValue.dwIndex<keyinfo->cValues; enumValue.dwIndex++){
        if ( ( retCode = get_data(&enumValue) ) == ERROR_SUCCESS ){
            print_data(enumValue, road->richkey);
        }else {
            fprintf(stderr,"Erreur %d get_data() : ",(int)retCode );
            fprintf(stderr,"dwIndex %d/%d  ",(int)enumValue.dwIndex,(int)keyinfo->cValues );
            if(retCode==ERROR_MORE_DATA){
                fprintf(stderr,"ERROR_MORE_DATA : maxvalue : %d\n",(int)keyinfo->cchMaxValue);
            }
            fprintf(stderr,"%d(%s)\n\n",(int)keyinfo->cbMaxValueData, road->str);
        }
    }
    RegCloseKey(enumValue.key);
    return 0;
    }

int get_data(EnumValue *enumValue){
    DWORD retCode;
    enumValue->lpcchValueName = (LPDWORD)MAX_VALUE_NAME;
    enumValue->lpcbData = (LPDWORD)MAX_VALUE_NAME;
    retCode = RegEnumValue(
            enumValue->key,
            enumValue->dwIndex,
            (LPSTR)enumValue->lpValueName,
            (LPDWORD)&enumValue->lpcchValueName,
            NULL,
            (LPDWORD)&enumValue->lpType,
            (LPBYTE)enumValue->lpData,
            (LPDWORD)&enumValue->lpcbData);
    return retCode;
    }

int flt_to_str(FILETIME *flt, char* str){
    SYSTEMTIME stUTC, stLocal;
    FileTimeToSystemTime(flt, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
    sprintf(str,"%d/%d/%d %dh%d",stLocal.wDay,stLocal.wMonth,stLocal.wYear,stLocal.wHour, stLocal.wMinute);
    return 0;
    }

int make_richkey(KeyInfo *keyinfo, Road *road){
    sprintf(road->richkey,"\"%s\",\"%s\",\"%d\",\"%s\",\"%d\"",
            road->shive,
            road->str ,
            (int)keyinfo->cValues,
            road->lwt,
            (int)keyinfo->cbSecurityDescriptor  );
    return 0;
    }

int print_data(EnumValue enumValue, char *rk){
    switch ((int)enumValue.lpType){
        case REG_SZ://A null-terminated string
            fprintf(stdout,"%s,\"REG_SZ\",\"%s\",\"%s\"\n",(char*)rk ,(char*)enumValue.lpValueName, (char*)enumValue.lpData);
            break;
        case REG_BINARY://Binary data in any form
            fprintf(stdout,"%s,\"REG_BINARY\",\"%s\",\"%d\"\n",(char*)rk,(char*)enumValue.lpValueName, (int)*enumValue.lpData);
            break;
        case REG_DWORD://A 32-bit number.
            fprintf(stdout,"%s,\"REG_DWORD\",\"%s\",\"%d\"\n",(char*)rk,(char*)enumValue.lpValueName, (int)*enumValue.lpData);
            break;
        case REG_DWORD_BIG_ENDIAN://A 32-bit number in big-endian format.
            fprintf(stdout,"%s,\"REG_DWORD_BIG_ENDIAN\",\"%s\",\"%d\"\n",(char*)rk,(char*)enumValue.lpValueName, (int)*enumValue.lpData);
            break;
        case REG_EXPAND_SZ://A null-terminated string for example, "%PATH%"
            fprintf(stdout,"%s,\"REG_EXPAND_SZ\",\"%s\",\"%s\"\n",(char*)rk,(char*)enumValue.lpValueName, (char*)enumValue.lpData);
            break;
        case REG_LINK://A null-terminated Unicode string that contains the target path of a symbolic link that was created by calling the RegCreateKeyEx function with REG_OPTION_CREATE_LINK.
            fprintf(stdout,"%s,\"REG_LINK\",\"%s\",\"%s\"\n",(char*)rk,(char*)enumValue.lpValueName, (char*)enumValue.lpData);
            break;
        case REG_MULTI_SZ://A sequence of null-terminated strings, terminated by an empty string (\0).
            fprintf(stdout,"%s,\"REG_MULTI_SZ\",\"%s\",\"%s\"\n",(char*)rk,(char*)enumValue.lpValueName, (char*)enumValue.lpData);
            break;
        case REG_NONE://No defined value type.
            fprintf(stdout,"%s,\"REG_NONE\",\"%s\",\"%s\"\n",(char*)rk,(char*)enumValue.lpValueName, (char*)enumValue.lpData);
            break;
        case REG_QWORD://A 64-bit
            fprintf(stdout,"%s,\"REG_QWORD\",\"%s\",\"%d\"\n",(char*)rk,(char*)enumValue.lpValueName, (int)*enumValue.lpData);
            break;
        default:
            fprintf(stdout,"%s,\"default\",\"%s\",\"%d\"\n",(char*)rk,(char*)enumValue.lpValueName, (int)enumValue.lpData);
            break;
        }
        return 0;
    }

int get_subKeys(Road *road, KeyInfo *keyinfo){
    KeyEnum key_enum;
    DWORD retCode=0;
    Road newRoad;
    if((retCode=RegOpenKeyEx( road->hive,road->str, 0, KEY_READ, &key_enum.key))!=0){
        fprintf(stderr,"Erreur get_subKeys, retCode : %d \n",(int)retCode);
    }else{

    TCHAR subkeyName[keyinfo->cbMaxSubKey+1];
    key_enum.lpName=subkeyName;

    key_enum.dwIndex = 0 ;
    while(key_enum.dwIndex < keyinfo->cSubKeys ){// && retCode==0){
        key_enum.lpcchName = (LPDWORD)keyinfo->cbMaxSubKey+1;
        retCode=RegEnumKeyEx(
            key_enum.key,
            key_enum.dwIndex,
            key_enum.lpName,
            (LPDWORD)&key_enum.lpcchName,
            NULL,
            NULL,
            NULL,
            &key_enum.lpftLastWriteTime);
        if( retCode==0){
            make_key(*road, key_enum, &newRoad);
            get_all(&newRoad);
        }else{
            fprintf(stderr,"Erreur RegEnumKeyEx, retCode : %d \n",(int)retCode);
            fprintf(stderr,"  keyinfo->cSubKeys : %d \n",(int)keyinfo->cSubKeys);
            fprintf(stderr,"  key_enum.dwIndex : %d \n\n",(int)key_enum.dwIndex);
        }
    key_enum.dwIndex=key_enum.dwIndex+1;
    }
    RegCloseKey(key_enum.key);
    }
    return EXIT_SUCCESS;
    }

int make_key(Road road, KeyEnum key_enum, Road *newRoad){
    size_t len= strlen(road.str);
    if(len==0 ){
        strcpy(newRoad->str, key_enum.lpName);
    }else if(len>0){
        strcpy(newRoad->str, road.str);
        strcat(newRoad->str, "\\");
        strcat(newRoad->str, key_enum.lpName);
    }
    newRoad->hive=road.hive;
    strcpy(newRoad->shive, road.shive);
    return EXIT_SUCCESS;
    }
