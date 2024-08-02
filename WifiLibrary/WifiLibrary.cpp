#include "pch.h"
#include <windows.h>
#include <wlanapi.h>
#include <objbase.h>
#include <wtypes.h>
#include <iostream>
#include<conio.h>
#include <sstream>
#include <iomanip>
#include <codecvt>
#include "WifiLibrary.h"

#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")

HANDLE hClient = NULL;
PWLAN_INTERFACE_INFO_LIST pIfList = NULL;

//convert string to wchar*
std::wstring convertToWideString(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

//create wlan xml profile
std::wstring createWLANProfileXML(const std::string& name, const std::string& ssid, const std::string& pass) {
    std::wostringstream xmlStream;
    xmlStream << L"<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">\n";
    xmlStream << L"<name>" << convertToWideString(name) << L"</name>\n";
    xmlStream << L"<SSIDConfig>\n<SSID>\n<hex>";
    for (char c : name) {
        xmlStream << std::hex << std::setw(2) << std::setfill(L'0') << (int)(unsigned char)c;
    }
    xmlStream << L"</hex>\n<name>" << convertToWideString(ssid) << L"</name>\n</SSID>\n<nonBroadcast>false</nonBroadcast>\n</SSIDConfig>\n";
    xmlStream << L"<connectionType>ESS</connectionType>\n";
    xmlStream << L"<connectionMode>auto</connectionMode>\n";
    xmlStream << L"<autoSwitch>false</autoSwitch>\n";
    xmlStream << L"<MSM>\n<security>\n<authEncryption>\n";
    xmlStream << L"<authentication>WPA2PSK</authentication>\n";
    xmlStream << L"<encryption>AES</encryption>\n";
    xmlStream << L"<useOneX>false</useOneX>\n</authEncryption>\n";
    xmlStream << L"<sharedKey>\n<keyType>passPhrase</keyType>\n";
    xmlStream << L"<protected>false</protected>\n";
    xmlStream << L"<keyMaterial>" << convertToWideString(pass) << L"</keyMaterial>\n";
    xmlStream << L"</sharedKey>\n<keyIndex>0</keyIndex>\n</security>\n</MSM>\n";
    xmlStream << L"</WLANProfile>";
    return xmlStream.str();
}

//add created profile to windows
 int create_profile(PWLAN_INTERFACE_INFO pIfInfo,const std::string& name, const std::string& ssid, const std::string& pass) {
    DWORD dwFlags = WLAN_PROFILE_USER;
    DWORD dwGrantedAccess = 0;
    std::wstring xmlString = createWLANProfileXML(name, ssid, pass);
    const LPCWSTR profileXml = xmlString.c_str();

    DWORD dwResult = WlanSetProfile(hClient,
        &pIfInfo->InterfaceGuid,
        dwFlags,
        profileXml,
        NULL,
        TRUE,
        NULL,
        &dwGrantedAccess);

    if (dwResult != ERROR_SUCCESS) {
        std::cerr << "WlanSetProfile failed with error: " << dwResult << std::endl;
        WlanFreeMemory(pIfList);
        WlanCloseHandle(hClient, NULL);
        return 1;
    }
}

int initWlanWithParams(DWORD dwMaxClient,DWORD dwCurVersion) {
    DWORD dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        std::cerr << "WlanOpenHandle failed with error: " << dwResult << std::endl;
        return 1;
    }

    dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
    if (dwResult != ERROR_SUCCESS) {
        std::cerr << "WlanEnumInterfaces failed with error: " << dwResult << std::endl;
        WlanCloseHandle(hClient, NULL);
        return 1;
    }
}

int initWlan() {
    DWORD dwMaxClient = 2;
    DWORD dwCurVersion = 0;
    DWORD dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        std::cerr << "WlanOpenHandle failed with error: " << dwResult << std::endl;
        return 1;
    }

    dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
    if (dwResult != ERROR_SUCCESS) {
        std::cerr << "WlanEnumInterfaces failed with error: " << dwResult << std::endl;
        WlanCloseHandle(hClient, NULL);
        return 1;
    }
    return 0;
}

PWLAN_INTERFACE_INFO_LIST getAllInterfaces() {
    return pIfList;
}

WLAN_INTERFACE_INFO* getInterface(int index) {
    PWLAN_INTERFACE_INFO pIfInfo = (WLAN_INTERFACE_INFO*)&pIfList->InterfaceInfo[index];
    return pIfInfo;
}

WifiNetworkList* getNetworks(PWLAN_INTERFACE_INFO pIfInfo) {
    PWLAN_AVAILABLE_NETWORK_LIST pBssList = NULL;
    WifiNetworkList* network_list = new WifiNetworkList();
    DWORD dwResult = WlanGetAvailableNetworkList(hClient,
        &pIfInfo->InterfaceGuid,
        0,
        NULL,
        &pBssList);
    WifiNetwork* networks = new WifiNetwork[pBssList->dwNumberOfItems];

    for (int j = 0; j < pBssList->dwNumberOfItems; j++) {
        PWLAN_AVAILABLE_NETWORK pBssEntry = (WLAN_AVAILABLE_NETWORK*)&pBssList->Network[j];
        memcpy(networks[j].wSSID, pBssEntry->dot11Ssid.ucSSID, 32);
        networks[j].dot11BssType = pBssEntry->dot11BssType;
        networks[j].uNumberOfBssids = pBssEntry->uNumberOfBssids;
        networks[j].bNetworkConnectable = pBssEntry->bNetworkConnectable;
        networks[j].uNumberOfPhyTypes = pBssEntry->uNumberOfPhyTypes;
        networks[j].bMorePhyTypes = pBssEntry->bMorePhyTypes;
        networks[j].bSecurityEnabled = pBssEntry->bSecurityEnabled;
    }
    network_list->networks = networks;
    network_list->size = pBssList->dwNumberOfItems;
    return network_list;
}

WLAN_INTERFACE_STATE getInterfaceState(PWLAN_INTERFACE_INFO pIfInfo) {
    return pIfInfo->isState;
}


int connectWlan(PWLAN_INTERFACE_INFO pIfInfo, const char* name) {

    std::wstring wideName = convertToWideString(name);
    const LPCWSTR profileName = wideName.c_str();

    WLAN_CONNECTION_PARAMETERS connParams;
    ZeroMemory(&connParams, sizeof(connParams));
    connParams.wlanConnectionMode = wlan_connection_mode_profile;
    connParams.strProfile = profileName;
    connParams.pDot11Ssid = NULL;
    connParams.dot11BssType = dot11_BSS_type_any;
    connParams.dwFlags = 0;

    DWORD dwResult = WlanConnect(hClient, &pIfInfo->InterfaceGuid, &connParams, NULL);
    if (dwResult != ERROR_SUCCESS) {
        std::cerr << "WlanConnect failed with error: " << dwResult << std::endl;
        WlanFreeMemory(pIfList);
        WlanCloseHandle(hClient, NULL);
        return 1;
    }
    return 0;
}

int connectAndCreateProfile(PWLAN_INTERFACE_INFO pIfInfo, const char* ssid, const char* pass) {
    create_profile(pIfInfo, ssid, ssid, pass);

    std::wstring wideName = convertToWideString(ssid);
    const LPCWSTR profileName = wideName.c_str();
    WLAN_CONNECTION_PARAMETERS connParams;
    ZeroMemory(&connParams, sizeof(connParams));
    connParams.wlanConnectionMode = wlan_connection_mode_profile;
    connParams.strProfile = profileName;
    connParams.pDot11Ssid = NULL;
    connParams.dot11BssType = dot11_BSS_type_any;
    connParams.dwFlags = 0;

    DWORD dwResult = WlanConnect(hClient, &pIfInfo->InterfaceGuid, &connParams, NULL);
    if (dwResult != ERROR_SUCCESS) {
        std::cerr << "WlanConnect failed with error: " << dwResult << std::endl;
        WlanFreeMemory(pIfList);
        WlanCloseHandle(hClient, NULL);
        return 1;
    }
    return 0;
}


