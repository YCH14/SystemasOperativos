#include <iostream>
#include <pthread.h>
#include <unistd.h>  // Para usar sleep()
#include <vector>

using namespace std;

// Variables globales compartidas
double saldo = 100000;  // Saldo inicial de 100,000
int peticion = 0;  // Petición de cargo o abono (0 significa ninguna operación en espera)
bool operacionExitosa = false;  // Resultado de la operación (true si fue exitosa)

// Variables para contar operaciones exitosas y fallidas
int operacionesExitosasServicio = 0;
int operacionesFallidasServicio = 0;
int operacionesExitosasCliente = 0;
int operacionesFallidasCliente = 0;

// Mutex para sincronización
pthread_mutex_t mutexOperacion = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condicionOperacion = PTHREAD_COND_INITIALIZER;

// Función del hilo "servicio"
void* servicio(void* arg) {
    while (true) {
        pthread_mutex_lock(&mutexOperacion);
        
        // Espera a que haya una petición
        while (peticion == 0) {
            pthread_cond_wait(&condicionOperacion, &mutexOperacion);
        }
        
        // Procesar la petición
        double nuevoSaldo = saldo + peticion;
        if (nuevoSaldo >= 0 && nuevoSaldo <= 1000000) {
            saldo = nuevoSaldo;
            operacionExitosa = true;
            operacionesExitosasServicio++;
        } else {
            operacionExitosa = false;
            operacionesFallidasServicio++;
        }
        
        // Reseteamos la petición
        peticion = 0;
        
        // Notificamos al cliente que la operación fue procesada
        pthread_cond_signal(&condicionOperacion);
        
        pthread_mutex_unlock(&mutexOperacion);
    }
    
    return nullptr;
}

// Función del hilo "cliente"
void* cliente(void* arg) {
    vector<int> montos = {500, -200, 300, -400, 2000, -1500, 3000, -1000, 100, -50, /*...*/};  // Ejemplo de montos
    int totalMovimientos = 10000;
    
    for (int i = 0; i < totalMovimientos; i++) {
        pthread_mutex_lock(&mutexOperacion);
        
        // Espera hasta que no haya ninguna operación en proceso
        while (peticion != 0) {
            pthread_cond_wait(&condicionOperacion, &mutexOperacion);
        }
        
        // Establecemos la petición
        peticion = montos[i % montos.size()];
        
        // Notificamos al servicio
        pthread_cond_signal(&condicionOperacion);
        
        // Esperamos la respuesta del servicio
        while (peticion != 0) {
            pthread_cond_wait(&condicionOperacion, &mutexOperacion);
        }
        
        // Registramos si la operación fue exitosa o fallida
        if (operacionExitosa) {
            operacionesExitosasCliente++;
        } else {
            operacionesFallidasCliente++;
        }
        
        pthread_mutex_unlock(&mutexOperacion);
    }
    
    return nullptr;
}

int main() {
    pthread_t hiloServicio, hiloCliente;
    
    // Crear hilos
    pthread_create(&hiloServicio, nullptr, servicio, nullptr);
    pthread_create(&hiloCliente, nullptr, cliente, nullptr);
    
    // Esperar a que el cliente termine
    pthread_join(hiloCliente, nullptr);
    
    // Imprimir resultados
    cout << "Cliente:" << endl;
    cout << "Operaciones exitosas: " << operacionesExitosasCliente << endl;
    cout << "Operaciones fallidas: " << operacionesFallidasCliente << endl;
    
    cout << "Servicio:" << endl;
    cout << "Operaciones exitosas: " << operacionesExitosasServicio << endl;
    cout << "Operaciones fallidas: " << operacionesFallidasServicio << endl;
    
    return 0;
}
