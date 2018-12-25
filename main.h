#define MAX_KEY_LENGTH 9999
#define MAX_VALUE_NAME 9999
#define STR_MAX  9999

typedef struct Road Road;
struct Road{
    HKEY hive;
    char shive[STR_MAX];// string hive
    char str[STR_MAX];//string key
    char richkey[STR_MAX];//string key enrichie "hive","key","security descriptor","last write time",
    char lwt[16];
};

typedef struct KeyInfo KeyInfo;
struct KeyInfo{
    HKEY key;
    TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
    DWORD    cbName;                   // size of name string
    TCHAR    achClass[MAX_PATH] ;  // buffer for class name
    DWORD    cchClassName ;  // size of class string
    DWORD    cSubKeys;               // number of subkeys
    DWORD    cbMaxSubKey;              // longest subkey size
    DWORD    cchMaxClass;              // longest class string
    DWORD    cValues;              // number of values for key
    DWORD    cchMaxValue;          // longest value name
    DWORD    cbMaxValueData;       // longest value data
    DWORD    cbSecurityDescriptor; // size of security descriptor
    FILETIME ftLastWriteTime;      // last write time
};

typedef struct KeyEnum KeyEnum;
struct KeyEnum{
    HKEY      key;     //hive
    DWORD     dwIndex;  //The index of the subkey to retrieve. This parameter should be zero for the first call to the RegEnumKeyEx function and then incremented for subsequent calls.
    TCHAR     lpName[MAX_KEY_LENGTH];   //A pointer to a buffer that receives the name of the subkey, including the terminating null character
    LPDWORD   lpcchName;
    LPDWORD   lpReserved;   //This parameter is reserved and must be NULL.
    LPSTR     lpClass[MAX_PATH];;  //A pointer to a buffer that receives the user-defined class of the enumerated subkey. This parameter can be NULL.
    LPDWORD   lpcchClass;   //A pointer to a variable that specifies the size of the buffer specified by the lpClass parameter, in characters. The size should include the terminating null character. If the function succeeds, lpcClass contains the number of characters stored in the buffer, not including the terminating null character. This parameter can be NULL only if lpClass is NULL.
    FILETIME lpftLastWriteTime;
};//ERROR_SUCCESS   ERROR_NO_MORE_ITEMS ERROR_MORE_DATA

typedef struct EnumValue EnumValue;
struct EnumValue{
    HKEY    key;
    DWORD   dwIndex;
    LPSTR   *lpValueName;
    LPDWORD lpcchValueName;
    LPDWORD lpReserved;
    LPDWORD lpType;
    LPBYTE  *lpData;
    LPDWORD lpcbData;
};
/**
    printf() entete du csv
**/
int print_title();

/**
    Recois l'element de argv correspondant a -h
    initialise la hkey road->hive
    initialise une chaine de caratere correspondant road->shive
**/
int set_hive(char *argv, Road *road);

/**
    Boucle sur les element de argv
    appel set_hive()
    initialise road->str si une clef a ete passer en argument (-k)
**/
int set_arg(int argc, char *argv[], Road *road);


/**
    Ouvre la clé contenu dans road
    Enumere les sous clé et recréer les chemin avec leur nom et road->str
    Rappel get_all() avec chaque sous clé
    Ferme la clé
**/
int get_subKeys(Road *road, KeyInfo *keyinfo);

/**
    switch sur le Type de la value
    printf de la value, du type et des data dans le bon format
**/
int print_data(EnumValue enumValue, char *rk);

/**
    Converti un FILETIME en SYSTEMETIME
    Passe de UTC a local
    creer la clé enrichie dans road->richkey (hive,clé, nombre de value, lastwritetime, security descriptor, )
**/
int make_richkey(KeyInfo *keyinfo, Road *road);

/**
    Recupere data et value de l'index dwIndex de la structure enumValue passé en parametre
**/
int get_data(EnumValue *enumValue);

/**
    Converti un FILETIME en chaine de caractere au format local,"dd/mm/yyyy hhhmm"
**/
int flt_to_str(FILETIME *flt, char* str);

/**
    Ouvre la clé contenu dans road
    Recupere ses info(nbr subkeys, nbr values, taille max des nom et key, date de derniere modif... )
    Ferme la clé
**/
int query_key(Road *road, KeyInfo *k);

/**
    Recois une clé et un hive
    Appel query_key() pour recuperer les info
    recupere les value si il y en a
    recupere les sous clé si il y en a
**/
int get_all(Road *road);

/**
    Ecrit la clé
    Ecrit sa date de derniere modification
    Boucle sur les value contenu dans une clé et appel get_data()
    appel print_data() pour ecrir le resultat
**/
int get_values(Road *road, KeyInfo *keyinfo);

/**
    Construit une sous clé newroad->str avec la clé road.str et la sous clé key_enum.lpName
**/
int make_key(const Road road, const KeyEnum key_enum, Road *newRoad);

