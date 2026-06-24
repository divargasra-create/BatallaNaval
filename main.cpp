#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iomanip>

/*tabla de codigos utiles*/
namespace ansi {
    const std::string limpiar          = "\033[2J\033[1;1H";
    const std::string reset            = "\033[0m";
    const std::string azul             = "\033[34m";
    const std::string blanco           = "\033[37m";
    const std::string rojo             = "\033[31m";
    const std::string verde            = "\033[32m";
    const std::string amarillo         = "\033[33m";
    const std::string ocultar_cursor   = "\033[?25l";
    const std::string mostrar_cursor   = "\033[?25h";
    // movimiento "\033[F;CH" con f=filas c=columnas
}

/*      estructura de la cantidad de barcos         */
struct Barco {
    int tamano;
    int cantidad;
};

/*      funciones de utilidad       */
std::string mover(int filas, int columnas) {
    return "\033[" + std::to_string(columnas) + ";" + std::to_string(filas) + "H";
}

/*      agrega un barco al tablero en la posicion y orientacion dadas       */
void anadirBarco(std::vector<std::vector<char>> & vector, int posx, int posy, int tamano, bool horizontal) {
    if (!horizontal) {
        for (int i = posx; i < tamano + posx; i++) {
            vector[posy][i] = 'B';
        }
    } else {
        for (int i = posy; i < tamano + posy; i++) {
            vector[i][posx] = 'B';
        }
    }
}

/*      verifica si es posible colocar un barco sin salirse ni pisar otro       */
bool anadirBarcoFlag(const std::vector<std::vector<char>> & vector, int posx, int posy, int tamano, bool horizontal) {
    if (horizontal && (posy + tamano) <= 10) {
        for (int i = posy; i < tamano + posy; i++) {
            if (vector[i][posx] != 'A') {
                return false;
            }
        }
        return true;
    } else if (!horizontal && (posx + tamano) <= 10) {
        for (int i = posx; i < tamano + posx; i++) {
            if (vector[posy][i] != 'A') {
                return false;
            }
        }
        return true;
    }
    return false;
}

/*      convertir coordenada estilo "A3" en indices fila/columna       */
bool parsearCoordenada(const std::string & coordenada, int & fila, int & columna) {
    if (coordenada.size() < 2 || coordenada.size() > 3) {
        return false;
    }
    char letra = toupper(coordenada[0]);
    if (letra < 'A' || letra > 'J') {
        return false;
    }
    std::string parteNumero = coordenada.substr(1);
    for (char c : parteNumero) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    int numero = std::stoi(parteNumero);
    if (numero < 1 || numero > 10) {
        return false;
    }
    fila    = letra - 'A';
    columna = numero - 1;
    return true;
}

/*      rellena el interior del tablero con los simbolos segun el estado de cada celda
        en el tablero de la maquina, ocultamos las 'B' para modo clasico (verMaquina=false)  */
void rellenarTablero(int x, int y, const std::vector<std::vector<char>> & vector, bool verMaquina) {
    int indexi = 0;
    for (int i = x + 1; i <= 19 + x; i += 2) {
        int indexj = 0;
        for (int j = y + 1; j <= 10 + y; j++) {
            char celda = vector[indexi][indexj];
            if (celda == 'A') {
                std::cout << mover(i, j) << ansi::azul << "*" << ansi::reset << std::flush;
            } else if (celda == 'B') {
                if (verMaquina) {
                    std::cout << mover(i, j) << ansi::blanco << "*" << ansi::reset << std::flush;
                } else {
                    std::cout << mover(i, j) << ansi::azul << "*" << ansi::reset << std::flush;
                }
            } else if (celda == 'R') {
                std::cout << mover(i, j) << ansi::rojo << "X" << ansi::reset << std::flush;
            } else if (celda == 'F') {
                std::cout << mover(i, j) << ansi::blanco << "o" << ansi::reset << std::flush;
            }
            indexj++;
        }
        indexi++;
    }
}

/*      dibuja el marco y el contenido de un tablero en la posicion (x,y) de la terminal       */
void proyectarTablero(int x, int y, const std::vector<std::vector<char>> & vector,
                      const std::string & titulo, bool verMaquina) {
    std::cout << mover(x - 1, y) << ansi::amarillo << titulo << ansi::reset << std::flush;

    for (int i = x; i <= 20 + x; i++) {
        std::cout << mover(i, y)      << ansi::blanco << "-" << ansi::reset << std::flush;
        std::cout << mover(i, y + 11) << ansi::blanco << "-" << ansi::reset << std::flush;
    }
    for (int i = y; i <= 10 + y; i++) {
        std::cout << mover(x,      i) << ansi::blanco << "|" << ansi::reset << std::flush;
        std::cout << mover(x + 20, i) << ansi::blanco << "|" << ansi::reset << std::flush;
    }

    rellenarTablero(x, y, vector, verMaquina);
}

/*      mensaje en la zona de instrucciones       */
void mostrarMensaje(const std::string & mensaje) {
    std::cout << mover(40, 16) << "                                                  " << std::flush;
    std::cout << mover(40, 16) << ansi::reset << mensaje << std::flush;
}

/*      limpia la linea de entrada de la terminal       */
void limpiarEntrada() {
    std::cout << mover(40, 18) << "                    " << std::flush;
}

/*      colocacion de la flota del jugador       */
void colocarFlotaJugador(std::vector<std::vector<char>> & tablero, std::vector<Barco> & flota) {
    for (Barco & barco : flota) {
        for (int n = 0; n < barco.cantidad; n++) {
            bool colocado = false;
            while (!colocado) {
                mostrarMensaje("Coloca barco de tamano " + std::to_string(barco.tamano) + " (ej: A3 H o A3 V)");
                limpiarEntrada();
                std::cout << mover(40, 18) << "> " << std::flush;
                std::string coordenada;
                char orientacionChar;
                std::cin >> coordenada >> orientacionChar;
                int fila, columna;
                if (!parsearCoordenada(coordenada, fila, columna)) {
                    mostrarMensaje("Coordenada invalida, intenta de nuevo");
                    continue;
                }
                bool horizontal = (toupper(orientacionChar) == 'H');
                if (anadirBarcoFlag(tablero, columna, fila, barco.tamano, horizontal)) {
                    anadirBarco(tablero, columna, fila, barco.tamano, horizontal);
                    colocado = true;
                } else {
                    mostrarMensaje("No hay espacio ahi, intenta de nuevo");
                }
            }
            proyectarTablero(5, 5, tablero, "  TU FLOTA  ", true);
        }
    }
}

/*      colocacion aleatoria de la flota de la maquina       */
void colocarFlotaMaquina(std::vector<std::vector<char>> & tablero, std::vector<Barco> & flota) {
    for (Barco & barco : flota) {
        for (int n = 0; n < barco.cantidad; n++) {
            bool colocado = false;
            while (!colocado) {
                int fila       = rand() % 10;
                int columna    = rand() % 10;
                bool horizontal = rand() % 2;
                if (anadirBarcoFlag(tablero, columna, fila, barco.tamano, horizontal)) {
                    anadirBarco(tablero, columna, fila, barco.tamano, horizontal);
                    colocado = true;
                }
            }
        }
    }
}

/*      disparo del jugador       */
bool turnoJugador(std::vector<std::vector<char>> & tableroMaquina) {
    bool disparado = false;
    while (!disparado) {
        mostrarMensaje("Tu turno. Ingresa coordenada de disparo (ej: B5):");
        limpiarEntrada();
        std::cout << mover(40, 18) << "> " << std::flush;
        std::string coordenada;
        std::cin >> coordenada;
        int fila, columna;
        if (!parsearCoordenada(coordenada, fila, columna)) {
            mostrarMensaje("Coordenada invalida, intenta de nuevo");
            continue;
        }
        if (tableroMaquina[fila][columna] == 'R' || tableroMaquina[fila][columna] == 'F') {
            mostrarMensaje("Ya disparaste ahi, elige otro lugar");
            continue;
        }
        if (tableroMaquina[fila][columna] == 'B') {
            tableroMaquina[fila][columna] = 'R';
            mostrarMensaje(ansi::rojo + "IMPACTO! Le diste a un barco enemigo!" + ansi::reset);
        } else {
            tableroMaquina[fila][columna] = 'F';
            mostrarMensaje(ansi::blanco + "Agua... fallaste." + ansi::reset);
        }
        disparado = true;
    }
    return true;
}

/*      disparo de la maquina       */
void turnoMaquina(std::vector<std::vector<char>> & tableroJugador) {
    int fila, columna;
    do {
        fila    = rand() % 10;
        columna = rand() % 10;
    } while (tableroJugador[fila][columna] == 'R' || tableroJugador[fila][columna] == 'F');

    if (tableroJugador[fila][columna] == 'B') {
        tableroJugador[fila][columna] = 'R';
        mostrarMensaje(ansi::rojo + "La maquina le dio a tu barco!" + ansi::reset);
    } else {
        tableroJugador[fila][columna] = 'F';
        mostrarMensaje(ansi::verde + "La maquina fallo." + ansi::reset);
    }
}

/*      cuenta cuantos barcos quedan vivos en un tablero       */
int contarBarcos(const std::vector<std::vector<char>> & tablero) {
    int cuenta = 0;
    for (const auto & fila : tablero) {
        for (char celda : fila) {
            if (celda == 'B') cuenta++;
        }
    }
    return cuenta;
}

/*      espera a que el jugador presione Enter       */
void esperarEnter() {
    std::cout << mover(40, 20) << "Presiona Enter para continuar..." << std::flush;
    std::cin.ignore();
    std::cin.get();
    std::cout << mover(40, 20) << "                                " << std::flush;
}

// ═══════════════════════════════════════════════════════════════════════════════
//   GESTION DE TIEMPOS EN CSV
// ═══════════════════════════════════════════════════════════════════════════════

const std::string ARCHIVO_CSV = "tiempos.csv";

/*      convierte segundos a formato legible "Xm Ys"       */
std::string formatearTiempo(double segundos) {
    int mins = static_cast<int>(segundos) / 60;
    int segs = static_cast<int>(segundos) % 60;
    std::ostringstream oss;
    if (mins > 0) oss << mins << "m ";
    oss << segs << "s";
    return oss.str();
}

/*      lee todos los tiempos de victorias guardados en el CSV
        el archivo tiene dos columnas: fecha,segundos
        si el archivo no existe, devuelve un vector vacio       */
std::vector<double> leerTiemposCSV() {
    std::vector<double> tiempos;
    std::ifstream archivo(ARCHIVO_CSV);
    if (!archivo.is_open()) {
        return tiempos;                         // primera vez: sin historial
    }
    std::string linea;
    std::getline(archivo, linea);               // saltar cabecera "fecha,segundos"
    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue;
        std::istringstream ss(linea);
        std::string fecha, valor;
        if (std::getline(ss, fecha, ',') && std::getline(ss, valor, ',')) {
            try {
                tiempos.push_back(std::stod(valor));
            } catch (...) {
                // linea malformada: se ignora
            }
        }
    }
    archivo.close();
    return tiempos;
}

/*      agrega una nueva entrada al CSV con la fecha actual y los segundos jugados
        si el archivo no existe lo crea con su cabecera       */
void guardarTiempoCSV(double segundos) {
    // comprobar si el archivo ya existe para saber si hay que escribir cabecera
    bool archivoNuevo = false;
    {
        std::ifstream prueba(ARCHIVO_CSV);
        archivoNuevo = !prueba.is_open();
    }

    std::ofstream archivo(ARCHIVO_CSV, std::ios::app);
    if (!archivo.is_open()) {
        return;     // no se pudo abrir, se omite silenciosamente
    }

    if (archivoNuevo) {
        archivo << "fecha,segundos\n";
    }

    // obtener fecha y hora actuales
    std::time_t ahora = std::time(nullptr);
    std::tm * tm_local = std::localtime(&ahora);
    char fechaBuf[20];
    std::strftime(fechaBuf, sizeof(fechaBuf), "%Y-%m-%d %H:%M:%S", tm_local);

    archivo << std::fixed << std::setprecision(2)
            << fechaBuf << "," << segundos << "\n";
    archivo.close();
}

// ═══════════════════════════════════════════════════════════════════════════════
//   FUNCION RECURSIVA PARA ENCONTRAR EL MENOR TIEMPO
// ═══════════════════════════════════════════════════════════════════════════════

/*      recorre el vector de tiempos de forma recursiva y devuelve el minimo.
        caso base  : un solo elemento -> es el minimo por definicion.
        caso recursivo: compara el primer elemento con el minimo del resto.   */
double menorTiempoRecursivo(const std::vector<double> & tiempos, int indice) {
    // caso base: ultimo elemento
    if (indice == static_cast<int>(tiempos.size()) - 1) {
        return tiempos[indice];
    }
    // caso recursivo: minimo entre el elemento actual y el resto
    double menorDelResto = menorTiempoRecursivo(tiempos, indice + 1);
    return (tiempos[indice] < menorDelResto) ? tiempos[indice] : menorDelResto;
}

// ═══════════════════════════════════════════════════════════════════════════════

int main() {
    srand(time(0));
    std::cout << ansi::limpiar << ansi::ocultar_cursor << std::flush;

    std::vector<Barco> flota = {{2, 2}, {3, 3}, {4, 1}};

    // ─── pantalla de instrucciones ───────────────────────────────────────────
    std::cout << ansi::amarillo  << mover(40, 8)  << "=== BATALLA NAVAL ===" << ansi::reset << std::flush;
    std::cout << ansi::reset     << mover(40, 10) << "instrucciones:"        << std::flush;
    std::cout << ansi::reset     << mover(40, 11) << "1. usted esta jugando contra una maquina."                              << std::flush;
    std::cout << ansi::reset     << mover(40, 12) << "2. tanto usted como la maquina tienen un tablero de 10x10."            << std::flush;
    std::cout << ansi::reset     << mover(40, 13) << "3. flota: 2 barcos de 2, 3 barcos de 3 y 1 barco de 4 espacios."      << std::flush;
    std::cout << ansi::reset     << mover(40, 14) << "4. las reglas son las mismas que en batalla naval tradicional."        << std::flush;
    std::cout << ansi::reset     << mover(40, 15) << "5. orientacion: H = horizontal, V = vertical."                         << std::flush;
    std::cout << ansi::reset     << mover(40, 16) << "6. X = impacto  o = fallo."                                            << std::flush;
    std::cout << ansi::reset     << mover(40, 18) << "Presiona Enter para comenzar..." << std::flush;
    std::cin.get();

    // ─── preparacion de tableros ─────────────────────────────────────────────
    std::cout << ansi::limpiar << std::flush;
    std::vector<std::vector<char>> tableroJugador(10, std::vector<char>(10, 'A'));
    std::vector<std::vector<char>> tableroMaquina(10, std::vector<char>(10, 'A'));

    proyectarTablero(5,  5, tableroJugador, "  TU FLOTA  ", true);
    proyectarTablero(28, 5, tableroMaquina, "  ENEMIGO   ", false);

    mostrarMensaje("Coloca tu flota:");
    colocarFlotaJugador(tableroJugador, flota);
    colocarFlotaMaquina(tableroMaquina, flota);

    // ─── inicio del cronometro ───────────────────────────────────────────────
    std::time_t tiempoInicio = std::time(nullptr);

    // ─── bucle principal de juego ─────────────────────────────────────────────
    bool gameover   = false;
    bool jugadorGano = false;

    while (!gameover) {
        // --- turno del jugador ---
        turnoJugador(tableroMaquina);
        proyectarTablero(28, 5, tableroMaquina, "  ENEMIGO   ", false);

        if (contarBarcos(tableroMaquina) == 0) {
            mostrarMensaje(ansi::verde + "GANASTE! Hundiste toda la flota enemiga!" + ansi::reset);
            jugadorGano = true;
            gameover    = true;
            break;
        }

        esperarEnter();

        // --- turno de la maquina ---
        turnoMaquina(tableroJugador);
        proyectarTablero(5, 5, tableroJugador, "  TU FLOTA  ", true);

        if (contarBarcos(tableroJugador) == 0) {
            mostrarMensaje(ansi::rojo + "PERDISTE! La maquina hundio toda tu flota." + ansi::reset);
            gameover = true;
            break;
        }

        esperarEnter();
    }

    // ─── fin del cronometro 
    std::time_t tiempoFin = std::time(nullptr);
    double segundosJugados = std::difftime(tiempoFin, tiempoInicio);

    //  pantalla final
    std::cout << mover(40, 22) << "Tiempo de esta partida: "
              << ansi::amarillo << formatearTiempo(segundosJugados) << ansi::reset << std::flush;

    if (jugadorGano) {
        // guardar tiempo en CSV solo si el jugador gano
        guardarTiempoCSV(segundosJugados);

        // leer todos los tiempos e imprimir el record
        std::vector<double> historial = leerTiemposCSV();
        if (!historial.empty()) {
            double record = menorTiempoRecursivo(historial, 0);
            std::cout << mover(40, 23)
                      << "Mejor tiempo historico: "
                      << ansi::verde << formatearTiempo(record) << ansi::reset
                      << "  (" << historial.size() << " partida(s) ganada(s))"
                      << std::flush;

            // destacar si esta partida es nuevo record
            if (segundosJugados <= record) {
                std::cout << mover(40, 24)
                          << ansi::amarillo << "*** NUEVO RECORD! ***" << ansi::reset
                          << std::flush;
            }
        }
    }

    std::cout << mover(40, 26) << "Gracias por jugar. Presiona Enter para salir." << std::flush;
    std::cin.ignore();
    std::cin.get();
    std::cout << ansi::mostrar_cursor << std::flush;

    return 0;
}
