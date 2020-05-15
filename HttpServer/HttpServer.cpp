#include <iostream>
#include <windows.h>
#include <string>

#define BUFSIZE 1000

using namespace std;

int main(int argc, char* argv[])
{
    WORD wVersionRequested = MAKEWORD(1, 1); // ��slo verze
    WSADATA data;           // Struktura s info. o knihovn�;    
    std::string text;       // P�ij�man� text
    sockaddr_in sockName;   // "Jm�no" soketu a ��slo portu
    sockaddr_in clientInfo; // Klient, kter� se p�ipojil 
    SOCKET mainSocket;      // Soket
    int port;               // ��slo portu
    char buf[BUFSIZE];      // P�ij�mac� buffer
    int size;               // Po�et p�ijat�ch a odeslan�ch byt�
    int addrlen;            // Velikost adresy vzd�len�ho po��ta�e
    int count = 0;          // Po�et p�ipojen�

    if (argc != 2)
    {
        cerr << "Syntaxe:\n\t" << argv[0]
            << " " << "port" << endl;
        return -1;
    }
    // P�iprav�me sokety na pr�ci
    if (WSAStartup(wVersionRequested, &data) != 0)
    {
        cout << "Nepoda�ilo se inicializovat sokety" << endl;
        return -1;
    }
    port = atoi(argv[1]);
    // Vytvo��me soket - viz minul� d�l
    if ((mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
        == INVALID_SOCKET)
    {
        cerr << "Nelze vytvo�it soket" << endl;
        WSACleanup();
        return -1;
    }
    // Zapln�me strukturu sockaddr_in
    // 1) Rodina protokol�;
    sockName.sin_family = AF_INET;
    // 2) ��slo portu, na kter�m �ek�me
    sockName.sin_port = htons(port);
    // 3) Nastaven� IP adresy lok�ln� s�ov� karty, p�es kterou 
    // je mo�no se p�ipojit. 
    // Nastav�me mo�nost p�ipojit se odkudkoliv. 
    sockName.sin_addr.s_addr = INADDR_ANY;
    //p�i�ad�me soketu jm�no
    if (bind(mainSocket, (sockaddr*)&sockName, sizeof(sockName))
        == SOCKET_ERROR)
    {
        cerr << "Probl�m s pojmenov�n�m soketu." << endl;
        WSACleanup();
        return -1;
    }
    // Vytvo��me frontu po�adavk� na spojen�. 
    // Vytvo��me frontu maxim�ln� velikosti 10 po�adavk�.
    if (listen(mainSocket, 10) == SOCKET_ERROR)
    {
        cerr << "Probl�m s vytvo�en�m fronty" << endl;
        WSACleanup();
        return -1;
    }
    do
    {
        // Pozna��m si velikost struktury clientInfo.
        // P�ed�m to funkci accept. 
        addrlen = sizeof(clientInfo);
        // Vyberu z fronty po�adavek na spojen�.
        // "client" je nov� soket spojuj�c� klienta se serverem.
        SOCKET client = accept(mainSocket, (sockaddr*)&clientInfo,
            &addrlen);
        int totalSize = 0;
        if (client == INVALID_SOCKET)
        {
            cerr << "Probl�m s p�ijet�m spojeni" << endl;
            WSACleanup();
            return -1;
        }
        // Zjist�m IP adresu klienta.
        cout << "N�kdo se p�ipojil z adresy: "
            << inet_ntoa((in_addr)clientInfo.sin_addr) << endl;
        // P�ijmu data. Ke komunikaci s klientem pou��v�m soket  "client"
        text = "";
        // P�ijmeme maxim�ln� 6 bytov� pozdrav. 
        while (totalSize != 6)
        {
            if ((size = recv(client, buf, BUFSIZE - 1, 0))
                == SOCKET_ERROR)
            {
                cerr << "Probl�m s p�ijet�m dat." << endl;
                WSACleanup();
                return -1;
            }
            cout << "P�ijato: " << size << endl;
            totalSize += size;
            text += buf;
        }
        cout << text;
        // Ode�lu pozdrav
        if ((size = send(client, "Nazdar\n", 8, 0)) == SOCKET_ERROR)
        {
            cerr << "Probl�m s odesl�n�m dat" << endl;
            WSACleanup();
            return -1;
        }
        cout << "Odesl�no: " << size << endl;
        // Uzav�u spojen� s klientem
        closesocket(client);
    } while (++count != 3);
    cout << "Kon��m" << endl;
    closesocket(mainSocket);
    WSACleanup();
    return 0;
}