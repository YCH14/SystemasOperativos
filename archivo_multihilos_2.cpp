/**
  * AUTORES:    Bruno Gabriel Carrada Alarcon
  *             Calzontzi Hernandez Yaretzi
  *             Contreras Colmenero Emilio Sebastian        
  * FECHA DE ELABORACIÃ“N: 26/06/2024
  * PROPÃ“SITO: SimulaciÃ³n del manejo de saldo en una cuenta bancaria 
    usando hilos POSIX en C.
  **/
#include <iostream>
#include <fstream>
#include <vector>
#include <pthread.h>
#include <windows.h> // Incluir esta biblioteca para usar Sleep

using namespace std;

const int NUM_CLIENTES = 100;
int saldo_cuenta = 10000; // Saldo inicial
pthread_mutex_t mutex_saldo = PTHREAD_MUTEX_INITIALIZER;

struct ParametrosSirviente {
    int cliente_id;
    vector<int> operaciones;
};

struct Resultados {
    int exitosos;
    int fallidos;
};

Resultados resultados[NUM_CLIENTES]; // Array para almacenar resultados de cada cliente

void* hilo_sirviente(void* arg) {
    ParametrosSirviente* params = static_cast<ParametrosSirviente*>(arg);
    int cliente_id = params->cliente_id;

    // Inicializar resultados
    resultados[cliente_id].exitosos = 0;
    resultados[cliente_id].fallidos = 0;

    for (size_t i = 0; i < params->operaciones.size(); ++i) {
        int monto = params->operaciones[i];

        pthread_mutex_lock(&mutex_saldo);
        if (saldo_cuenta + monto >= 0) {
            saldo_cuenta += monto;
            resultados[cliente_id].exitosos++;
            cout << "Cliente " << cliente_id << ": Operación exitosa de " << monto << endl;
        } else {
            resultados[cliente_id].fallidos++;
            cout << "Cliente " << cliente_id << ": Operación fallida de " << monto << endl;
        }
        pthread_mutex_unlock(&mutex_saldo);
        
        Sleep(10); // Cambiado de usleep(10000) a Sleep(10)
    }

    return nullptr;
}

int main() {
    ifstream archivo_montos("movimientos.txt");
    if (!archivo_montos) {
        cerr << "Error al abrir el archivo movimientos.txt" << endl;
        return 1;
    }

    vector<int> operaciones;
    int monto;
    while (archivo_montos >> monto) {
        operaciones.push_back(monto);
    }
    archivo_montos.close();

    pthread_t hilos[NUM_CLIENTES];
    for (int i = 0; i < NUM_CLIENTES; ++i) {
        ParametrosSirviente* params = new ParametrosSirviente;
        params->cliente_id = i;
        params->operaciones = operaciones; // Cada cliente recibe la misma lista de operaciones

        pthread_create(&hilos[i], nullptr, hilo_sirviente, params);
    }

    for (int i = 0; i < NUM_CLIENTES; ++i) {
        pthread_join(hilos[i], nullptr);
    }

    // Imprimir resultados finales
    cout << "Resultados finales:\n";
    for (int i = 0; i < NUM_CLIENTES; ++i) {
        cout << "Cliente " << i << ": Exitosos = " << resultados[i].exitosos
             << ", Fallidos = " << resultados[i].fallidos << endl;
    }

    cout << "Saldo final: " << saldo_cuenta << endl;

    return 0;
}



