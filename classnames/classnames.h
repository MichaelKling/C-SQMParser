#ifndef _CLASSNAMES_H_
#define _CLASSNAMES_H_              
        void classnamesCreateSites();
        void classnamesCreateRoles();
        void classnamesCreateClassnamesMen();
        void classnamesCreateClassnamesVehicle();
        
        void classnamesCreateAll();
        
        typedef struct roles {
            boolean Commander;
            boolean Driver;
            boolean Gunner;
        } Roles;
        
        char *classnamesGetNatoAlphabet(int number);
        int classnamesGetNatoAlpabetSize();
        void classnamesGetPlayerRoles(char *role, Roles *roles);
        char *classnamesGetRank(char *rank);
        char *classnamesGetRankShort(char *rank);
#endif /* _CLASSNAMES_H_ */        