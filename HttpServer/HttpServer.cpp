#include <iostream>
#include <windows.h>
#include <string>

#define BUFSIZE 1000

using namespace std;

int main(int argc, char* argv[])
{
    WORD wVersionRequested = MAKEWORD(1, 1); // Èíslo verze
    WSADATA data;           // Struktura s info. o knihovnì;    
    std::string text;       // Pøijímaný text
    sockaddr_in sockName;   // "Jméno" soketu a èíslo portu
    sockaddr_in clientInfo; // Klient, který se pøipojil 
    SOCKET mainSocket;      // Soket
    int port;               // Èíslo portu
    char buf[BUFSIZE];      // Pøijímací buffer
    int size;               // Poèet pøijatých a odeslaných bytù
    int addrlen;            // Velikost adresy vzdáleného poèítaèe
    int count = 0;          // Poèet pøipojení

    if (argc != 2)
    {
        cerr << "Syntaxe:\n\t" << argv[0]
            << " " << "port" << endl;
        return -1;
    }
    // Pøipravíme sokety na práci
    if (WSAStartup(wVersionRequested, &data) != 0)
    {
        cout << "Nepodaøilo se inicializovat sokety" << endl;
        return -1;
    }
    port = atoi(argv[1]);
    // Vytvoøíme soket - viz minulý díl
    if ((mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
        == INVALID_SOCKET)
    {
        cerr << "Nelze vytvoøit soket" << endl;
        WSACleanup();
        return -1;
    }
    // Zaplníme strukturu sockaddr_in
    // 1) Rodina protokolù;
    sockName.sin_family = AF_INET;
    // 2) Èíslo portu, na kterém èekáme
    sockName.sin_port = htons(port);
    // 3) Nastavení IP adresy lokální síové karty, pøes kterou 
    // je možno se pøipojit. 
    // Nastavíme možnost pøipojit se odkudkoliv. 
    sockName.sin_addr.s_addr = INADDR_ANY;
    //pøiøadíme soketu jméno
    if (bind(mainSocket, (sockaddr*)&sockName, sizeof(sockName))
        == SOCKET_ERROR)
    {
        cerr << "Problém s pojmenováním soketu." << endl;
        WSACleanup();
        return -1;
    }
    // Vytvoøíme frontu požadavkù na spojení. 
    // Vytvoøíme frontu maximální velikosti 10 požadavkù.
    if (listen(mainSocket, 10) == SOCKET_ERROR)
    {
        cerr << "Problém s vytvoøením fronty" << endl;
        WSACleanup();
        return -1;
    }
    do
    {
        // Poznaèím si velikost struktury clientInfo.
        // Pøedám to funkci accept. 
        addrlen = sizeof(clientInfo);
        // Vyberu z fronty požadavek na spojení.
        // "client" je nový soket spojující klienta se serverem.
        SOCKET client = accept(mainSocket, (sockaddr*)&clientInfo,
            &addrlen);
        int totalSize = 0;
        if (client == INVALID_SOCKET)
        {
            cerr << "Problém s pøijetím spojeni" << endl;
            WSACleanup();
            return -1;
        }
        // Zjistím IP adresu klienta.
        cout << "Nìkdo se pøipojil z adresy: "
            << inet_ntoa((in_addr)clientInfo.sin_addr) << endl;
        // Pøijmu data. Ke komunikaci s klientem používám soket  "client"
        text = "";
        // Pøijmeme maximálnì 6 bytový pozdrav. 
        while (totalSize != 6)
        {
            if ((size = recv(client, buf, BUFSIZE - 1, 0))
                == SOCKET_ERROR)
            {
                cerr << "Problém s pøijetím dat." << endl;
                WSACleanup();
                return -1;
            }
            cout << "Pøijato: " << size << endl;
            totalSize += size;
            text += buf;
        }
        cout << text;
        // Odešlu pozdrav
        if ((size = send(client, "Nazdar\n", 8, 0)) == SOCKET_ERROR)
        {
            cerr << "Problém s odesláním dat" << endl;
            WSACleanup();
            return -1;
        }
        cout << "Odesláno: " << size << endl;
        // Uzavøu spojení s klientem
        closesocket(client);
    } while (++count != 3);
    cout << "Konèím" << endl;
    closesocket(mainSocket);
    WSACleanup();
    return 0;
}