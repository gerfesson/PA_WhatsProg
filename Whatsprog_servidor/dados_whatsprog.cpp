#include <iostream>
#include <algorithm>
#include <string.h>
#include <list>
#include "dados_whatsprog.h"
#include "mensageiro.h"
#include "winsocket.h"

typedef list<Usuario> list_Usr;
typedef list_Usr::iterator iter_Usr;
typedef list<Mensagem> list_Msg;
typedef list_Msg::iterator iter_Msg;
using namespace std;

// VAR GLOBAIS

// O socket de conexoes
tcp_winsocket_server c;
// A lista de clientes conectados
//list_Client serv.LU;
// O servidor (incluindo a lista de usuarios cadastrados, mas nao obrigatoriamente conectados)
Servidor serv;
// O flag que indica que o software deve encerrar todas as threads
bool fim = false;
//INICIO MENSAGEM.CPP

bool Mensagem::setId(uint32_t I)
{
    if (I<=0)
    {
        id = 0;
        return false;
    }
    id=I;
    return true;
}

bool Mensagem::setRemetente(const string &R)
{
    if (R.size()<TAM_MIN_NOME_USUARIO || R.size()>TAM_MAX_NOME_USUARIO)
    {
        remetente = "";
        return false;
    }
    remetente=R;
    return true;
}

bool Mensagem::setDestinatario(const string &D)
{
    if (D.size()<TAM_MIN_NOME_USUARIO || D.size()>TAM_MAX_NOME_USUARIO)
    {
        destinatario = "";
        return false;
    }
    destinatario = D;
    return true;
}

bool Mensagem::setTexto(const string &T)
{
    if (T.size()==0)
    {
        texto = "";
        return false;
    }
    texto=T;
    return true;
}

bool Mensagem::setStatus(MsgStatus S)
{
    if (S!=MSG_ENVIADA && S!=MSG_RECEBIDA && S!=MSG_ENTREGUE && S!=MSG_LIDA && S!=MSG_INVALIDA)
    {
        status = MSG_INVALIDA;
        return false;
    }
    status=S;
    return true;
}

//FIM MENSAGEM.CPP

//INICIO _.CPP

/** FUNCIONANDO KRAIIII **/
/** AS VEZES NEMMMMM TA **/
void Usuario::setMsg(int32_t id, const string &rem, const string &dest, const string &txt, MsgStatus s)
{
    Mensagem m;
    if(m.setId(id) && m.setRemetente(rem) && m.setDestinatario(dest) && m.setTexto(txt) && m.setStatus(s))
    {
        this->LM.push_back(m);
    }
    else
    {
        cout << "Erro ao armazenar mensagem do remetente: " << this->getLogin() << "\n";
    }
}

/** CASO DE USO I **/
void Servidor::new_case1(tcp_winsocket &t)
{
    WINSOCKET_STATUS iResult;
    string usr_login, usr_senha;
    iter_Usr i;
    // Já sabe que é para fazer um CMD_NEW_USR dentro da thread e depois le o nome de usuario e a senha do cliente
    iResult = t.read_string(usr_login,TEMPO_MAXIMO*1000);
    if (iResult == SOCKET_ERROR)
    {
        cerr << "Erro na leitura do nome de login de um cliente que se conectou.\n";
        t.close();
    }
    else
    {
        // Procura se ja existe um cliente cadastrado com o mesmo login
        i = find(serv.LU.begin(), serv.LU.end(), usr_login);
        if (i != serv.LU.end())
        {
            t.write_int(CMD_LOGIN_INVALIDO);
            t.close();
        }
        else
        {
            iResult = t.read_string(usr_senha, TEMPO_MAXIMO*1000);
            if (iResult == SOCKET_ERROR)
            {
                cerr << "Erro na leitura da senha de um cliente que se conectou.\n";
                t.close();
            }
            else
            {
                Usuario novo;
                novo.setLogin(usr_login);
                novo.setSenha(usr_senha);
                novo.setSocket(t);
                this->LU.push_back(novo);
                this->LC.push_back(novo);
                cout << "\ntamanho do vetor de user: " << this->LU.size() << "\n";
                cout << "\ntamanho do vetor de clientes: " << this->LC.size() << "\n";
                cout << "Usuário " << usr_login << " conectado.\n";
                t.write_int(CMD_LOGIN_OK);
            }
        }
    }
}

/** TEORICAMENTE TA CERTA, MAS FALTA ENVIAR AS MENSAGENS QUE TEM NO BUFFER DO CLIENTE **/
/** CASO DE USO II **/
void Servidor::new_case2(tcp_winsocket &t)
{
    WINSOCKET_STATUS iResult;
    string usr_login, usr_senha;
    iter_Usr i;
    iResult = t.read_string(usr_login,TEMPO_MAXIMO*1000);
    cout << usr_login;
    if (iResult == SOCKET_ERROR)
    {
        cerr << "Erro na leitura do nome de login de um cliente que se conectou.\n";
        t.close();
    }
    else
    {
        cout << "\ntamanho do vetor de user: " << this->LU.size() << "\n";
        cout << "\ntamanho do vetor de clientes: " << this->LC.size() << "\n";
        // Procura se ja existe um cliente cadastrado(on/off) com o mesmo login
        i = find(serv.LU.begin(), serv.LU.end(), usr_login);
        if (i != serv.LU.end())
        {
            if(i->getSocket().connected())
            {
                t.write_int(CMD_LOGIN_INVALIDO);
                cout << "legal";
                t.close();
            }
            else
            {
                iResult = t.read_string(usr_senha, TEMPO_MAXIMO*1000);
                if (iResult == SOCKET_ERROR)
                {
                    cerr << "Erro na leitura da senha de um cliente que se conectou.\n";
                    cout << "oi1";
                    t.close();
                }
                else
                {
                    if(usr_senha ==  i->getSenha())
                    {
                        t.write_int(CMD_LOGIN_OK);
                        i->setSocket(t);
                        this->LC.push_back(*i);
                        cout << "oifaces";
                        //FALTA ENVIAR AS MENSAGENS DESTINADAS A ELE
                    }
                    else
                    {
                        t.write_int(CMD_LOGIN_INVALIDO);
                        cout << "chamafio";
                        t.close();
                    }
                }
            }
        }
        else
        {
            t.write_int(CMD_LOGIN_INVALIDO);
            t.close();
        }
    }
}

/** CASO DE USO III **/
void Servidor::new_case3(tcp_winsocket &t)
{
    WINSOCKET_STATUS iResult;
    string usr_login, usr_senha;
    iter_Usr i;
    iResult = t.read_string(usr_login,TEMPO_MAXIMO*1000);
    if (iResult == SOCKET_ERROR)
    {
        cerr << "Erro na leitura do nome de login de um cliente que se conectou.\n";
        t.close();
    }
    else
    {
        // Procura se ja existe um cliente cadastrado(on/off) com o mesmo login
        i = find(serv.LU.begin(), serv.LU.end(), usr_login);
        if (i != serv.LU.end())
        {
            if(i->getSocket().connected())
            {
                t.write_int(CMD_LOGIN_INVALIDO);
                t.close();
            }
            else
            {
                iResult = t.read_string(usr_senha, TEMPO_MAXIMO*1000);
                if (iResult == SOCKET_ERROR)
                {
                    cerr << "Erro na leitura da senha de um cliente que se conectou.\n";
                    t.close();
                }
                else
                {
                    if(usr_senha ==  i->getSenha())
                    {
                        t.write_int(CMD_LOGIN_OK);
                        //FALTA ENVIAR AS MENSAGENS DESTINADAS A ELE
                    }
                    else
                    {
                        t.write_int(CMD_LOGIN_INVALIDO);
                        t.close();
                    }
                }
            }
        }
        else
        {
            t.write_int(CMD_LOGIN_INVALIDO);
            t.close();
        }
    }
}

/** CASO DE USO IV **/
void Servidor::new_case4(tcp_winsocket &t, Usuario &u)
{
    WINSOCKET_STATUS iResult;
    iter_Usr i;
    iter_Msg j;
    for (j=serv.buffer.begin(); j!=serv.buffer.end(); j++)
    {
        if((j->getDestinatario() == u.getLogin()) && (j->getRemetente() != u.getLogin()) &&
        (j->getStatus() == MSG_RECEBIDA))
        {
            if(u.getSocket().connected())
            {
                iResult = u.getSocket().write_int(CMD_NOVA_MSG);
                if(iResult != SOCKET_ERROR)
                {
                    if(u.getSocket().connected()) iResult = u.getSocket().write_string(j->getRemetente());
                }
                else
                {
                    u.getSocket().shutdown(); //para de receber solicitações
                }
                if(iResult != SOCKET_ERROR)
                {
                    if(u.getSocket().connected()) iResult = u.getSocket().write_string(j->getTexto());
                }
                else
                {
                    u.getSocket().shutdown(); //para de receber solicitações
                }
                j->setStatus(MSG_ENTREGUE);
                t.write_int(CMD_MSG_ENTREGUE);
                t.write_int(j->getId());
            }
        }
    }
}

/** CASO DE USO VII **/
void Servidor::new_case7(Usuario &i)
{
    iter_Usr j;
    cout << i.getLogin();
    if(i.getSocket().connected())
    {
        j = find(serv.LU.begin(), serv.LU.end(), i.getLogin());
        if (j != serv.LU.end())
        {
            j->getSocket().shutdown();
        }
        cout << "\ntamanho do vetor de user: " << this->LU.size() << "\n";
        cout << "\ntamanho do vetor de clientes: " << this->LC.size() << "\n";
        cout << "\n logout \n";
    }
}

//THREAD QUE DESEMPENHA AS FUNÇOES DO SERVIDOR
DWORD WINAPI servidor(LPVOID lpParameter)
{
    tcp_winsocket t;
    winsocket_queue f;
    WINSOCKET_STATUS iResult;

    list_Usr usr;

    string usuario, msg;
    int32_t comando;
    iter_Usr i;
    iter_Msg j;
    int legal = 0;

    while (!fim)
    {
        legal = 0;
        // Inclui na fila de sockets para o select todos os sockets que eu
        // quero monitorar para ver se houve chegada de dados
        f.clean();
        fim = false; //limpa a fila de sockets
        cout << "\n sim \n";
        if (!(fim = !c.accepting())) //
        {
            f.include(c); //inclui o socket de conexão na fila(como por ex, usuarios que tentam se cadastrar)
            legal++;
            cout << "\n dnv \n";
            cout << "\n "<<c<<"\n";
            cout << "\n"<<legal<<"\n";
            for (i=serv.LC.begin(); i!=serv.LC.end(); i++)
            {
                if (i->getSocket().connected())
                {
                    legal++;
                    cout << "incluiu";
                    cout << "\n nao \n";
                    cout << "\n"<<legal<<"\n";
                    f.include(i->getSocket());//inclui todos os clientes conectados da lista, na fila de espera de funçoes a serem feitas com eles
                }
            }
        }
        // Espera que chegue alguma dados em qualquer dos sockets da fila
        iResult = f.wait_read(TEMPO_MAXIMO*1000);
        if (iResult==SOCKET_ERROR)
        {
            if (!fim) cerr << "Erro na espera por alguma atividade\n";
            fim = true;
        }
        if (!fim)
        {
            cout << "\n foi \n";
            if (iResult!=0)
            {
                /** ------------------- FALTA ESSA PARTE DE MSG ------------------- **/
                // Não saiu por timeout: houve atividade em algum socket da fila
                // Testa em qual socket houve atividade.

                // Primeiro, testa os sockets dos clientes
                for (i=serv.LC.begin(); i!=serv.LC.end(); i++)
                {
                    if (i->getSocket().connected() && f.had_activity(i->getSocket()))
                    {
                        int32_t top;
                        if(i->getSocket().read_int(top, TEMPO_MAXIMO*1000 != SOCKET_ERROR))
                        {
                            switch(top)
                            {
                                case CMD_LOGOUT_USER:
                                    serv.new_case7(*i);
                                    i->getSocket().close();
                                    i = serv.LC.erase(i);
                            }
                        }
                        /*Usuario u;
                        u.setLogin(i->getLogin());
                        u.setSenha(i->getSenha());
                        u.setSocket(i->getSocket());
                        serv.new_case4(t, u);*/
                    }
                }
                cout << "\ntamanho do vetor de user: " << serv.LU.size() << "\n";
                cout << "\ntamanho do vetor de clientes: " << serv.LC.size() << "\n";

                /** ------------------- FALTA ADICIONAR OS OUTROS CASOS DE USO NO SWITCH ------------------- **/
                // Depois, testa se houve atividade no socket de conexao
                cout << "opabb";
                if (f.had_activity(c))
                {
                    cout << "tem";
                    if (c.accept(t) != SOCKET_OK)
                    {
                        cout << "toma no cuuuuuuu";
                        cerr << "Não foi possível estabelecer uma conexao\n";
                        fim = true;
                    }
                    if (!fim)
                    {
                        cout << "simsenho";
                        iResult = t.read_int(comando, TEMPO_MAXIMO*1000);
                        cout << comando;
                        if(iResult == SOCKET_ERROR)
                        {
                            cerr << "Erro ao tentar ler o comando fornecido pelo cliente.\n";
                            t.close();
                        }
                        else
                        {
                            switch(comando){
                                case CMD_NEW_USER:
                                    cout << "1";
                                    serv.new_case1(t);
                                    break;
                                case CMD_LOGIN_USER:
                                    cout << "\ntamanho do vetor de user: " << serv.LU.size() << "\n";
                                    cout << "\ntamanho do vetor de clientes: " << serv.LC.size() << "\n";
                                    cout << "2";
                                    serv.new_case2(t);
                                    break;
                                case CMD_NOVA_MSG:
                                    cout << "3";
                                    serv.new_case3(t);
                                    break;
                                case CMD_LOGOUT_USER:
                                    cout << "4";
                                    //serv.new_case7(t);
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
            }
            else
            {
                // Saiu poe timeout: não houve atividade em nenhum socket da fila
                if (serv.LC.empty())
                {
                    cout << "Servidor inativo hah " << TEMPO_MAXIMO << " segundos...\n";
                }
            }

            // Depois de testar a chegada de dados em todos os sockets,
            // elimina da lista de sockets as conexoes que foram fechadas porque houve
            // falha na comunicacao ou porque se desligaram
            //usr = serv.LU;
            for (i=serv.LC.begin(); i!=serv.LC.end(); i++)
            {
                cout << "5";
                while ( i!=serv.LC.end() && !(i->getSocket().connected()) )
                {
                    cout << "6";
                    i->getSocket().close();
                    i = serv.LC.erase(i);
                }
            }
            cout << "7";
        }
    }
    return 0;
}

int main(void)
{
  WSADATA wsaData;
  string msg;

  // All processes that call Winsock functions must first initialize the use of the Windows Sockets DLL (WSAStartup)
  // before making other Winsock functions calls
  // The MAKEWORD(2,2) parameter of WSAStartup makes a request for version 2.2 of Winsock on the system
  WINSOCKET_STATUS iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (iResult != 0) {
    cerr << "WSAStartup failed: " << iResult << endl;
    exit(1);
  }

  if (c.listen(PORTA_TESTE) != SOCKET_OK) {
    cerr << "Não foi possível abrir o socket de controle\n";
    exit(1);
  }

  // Cria a thread que recebe e reenvia as mensagens
  HANDLE tHandle = CreateThread(NULL, 0, servidor, NULL , 0, NULL);
  if (tHandle == NULL)
  {
    cerr << "Problema na criação da thread: " << GetLastError() << endl;
    exit(1);
  }


  while (!fim)
  {
    /*{
      cout << "Mensagem para todos os clientes [max " << TAM_MAX_MSG_STRING << " caracteres, FIM para terminar]: ";
      cin >> ws;
      getline(cin,msg);
    } while (msg.size()==0 || msg.size()>TAM_MAX_MSG_STRING);
    if (!fim) fim = (msg=="FIM");
    if (!fim)
    {
         //envie_msg("SERVIDOR","ALL",msg);
        //for (iter_Usr i=serv.LU.begin(); i!=serv.LU.end(); i++) cout << i->getLogin();
    }*/
  }

  // Desliga os sockets
  cout << "Encerrando o socket de conexoes\n";
  c.shutdown();
  for (iter_Usr i=serv.LU.begin(); i!=serv.LU.end(); i++)
  {
    cout << "Encerrando o socket do cliente " << i->getLogin() << endl;
    i->getSocket().shutdown();
  }
  // Espera pelo fim da thread do servidor (máximo de 5 segundos)
  cout << "Aguardando o encerramento da outra thread...\n";
  WaitForSingleObject(tHandle, 5000);
  // Encerra na "força bruta" a thread do servidor caso ela não tenha terminado sozinha
  // (ou seja, a função WaitForSingleObject tenha saído por timeout)
  TerminateThread(tHandle,0);
  // Encerra o handle da thread
  CloseHandle(tHandle);
  // Enderra o socket
  c.close();
  for (iter_Usr i=serv.LU.begin(); i!=serv.LU.end(); i++) i->getSocket().close();

  /* call WSACleanup when done using the Winsock dll */
  WSACleanup();
}
