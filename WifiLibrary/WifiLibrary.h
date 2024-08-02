// MathLibrary.h - Contains declarations of math functions
#pragma once

#ifdef WIFILIBRARY_EXPORTS
#define WIFILIBRARY_API __declspec(dllexport)
#else
#define WIFILIBRARY_API __declspec(dllimport)
#endif
#include <windows.h>
#include <wlanapi.h>
#include <iostream>

//structure contains information about an available wireless network.
struct WIFILIBRARY_API WifiNetwork {
    //SSID of the visible wireless network.
    UCHAR wSSID[32];
    //value that specifies whether the network is infrastructure or ad hoc.
    DOT11_BSS_TYPE dot11BssType;
    //Indicates the number of BSSIDs in the network.
    ULONG uNumberOfBssids;
    //Indicates whether the network is connectable or not. If set to TRUE, the network is connectable, otherwise the network cannot be connected to.
    BOOL bNetworkConnectable;
    //The number of PHY types supported on available networks. The maximum value of uNumberOfPhyTypes is WLAN_MAX_PHY_TYPE_NUMBER, which has a value of 8. If more than WLAN_MAX_PHY_TYPE_NUMBER PHY types are supported, bMorePhyTypes must be set to TRUE.
    ULONG uNumberOfPhyTypes;
    //Specifies if there are more than WLAN_MAX_PHY_TYPE_NUMBER PHY types supported.
    BOOL bMorePhyTypes;
    //Indicates whether security is enabled on the network. A value of TRUE indicates that security is enabled, otherwise it is not.
    BOOL bSecurityEnabled;
};

//structure contains an array of information about available networks.
struct WIFILIBRARY_API WifiNetworkList {
    //Contains the number of items in the Network member.
    int size;
    // containing network information.
    WifiNetwork* networks;
};

// init wlan interfaces and open wlan handle
extern "C" WIFILIBRARY_API int initWlan();

// init wlan interfaces and open wlan handle with max client and version
extern "C" WIFILIBRARY_API int initWlanWithParams(DWORD dwMaxClient, DWORD dwCurVersion);

// get interface by index
extern "C" WIFILIBRARY_API PWLAN_INTERFACE_INFO getInterface(int index);

// get all wlan network
extern "C" WIFILIBRARY_API WifiNetworkList* getNetworks(PWLAN_INTERFACE_INFO pIfInfo);

//get state of selected interfaces
extern "C" WIFILIBRARY_API WLAN_INTERFACE_STATE getInterfaceState(PWLAN_INTERFACE_INFO pIfInfo);

//connect to wifi if profile is already created
extern "C" WIFILIBRARY_API int connectWlan(PWLAN_INTERFACE_INFO pIfInfo, const char* name);

//create profile and connect to wifi
extern "C" WIFILIBRARY_API int connectAndCreateProfile(PWLAN_INTERFACE_INFO pIfInfo, const char* ssid, const char* pass);