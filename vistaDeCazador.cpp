#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <stdlib.h>

using namespace std;

// #variables y pre-def globales
#define colorMAX 65536
int colorVisitado[colorMAX] = {0};

// #estructuras
struct Intervalo
{
    int inicio = -1, final = -1;
};
struct Punto
{
    int x = -1, y = -1;
};
struct Circulo
{
    int id = -1;
    Punto centro;
    Intervalo abscisas;
    Intervalo ordenadas;
    int color = 0;
    int circunferencia = 0;
    int diametro = 0;
    int radio = 0;
    float pi = 0.0;
    int AND = 0; // Area Normalizada Discreta

    void encontrarCentro()
    {
        int tempX = (this->abscisas.inicio + this->abscisas.final) / 2;
        int tempY = (this->ordenadas.inicio + this->ordenadas.final) / 2;
        this->centro.x = (tempX > 0) ? tempX : 0;
        this->centro.y = (tempY > 0) ? tempY : 0;
    }
    void encontrarDiametro()
    {
        this->diametro = (this->abscisas.final - this->abscisas.inicio + 1);
    }
    void encontrarPi()
    {
        this->pi = ((float)this->circunferencia / this->diametro);
    }
    void encontrarRadio()
    {
        this->radio = (this->diametro / 2) + 1;
    }
};
class Imagen
{
private:
    int ancho, alto;
    int colorMaximo;
    float piNormalizado;

    int mejorArea;
    int menorMunicion;
    int actualMunicionGastada;

    bool *mejorEleccion;
    bool *circulosElegidos;

    int **pixeles;
    int **pixelesSoloCircunferencia;

    Circulo *circulos;

public:
    // #constructores y destructores
    Imagen(int M) : ancho(0), alto(0), colorMaximo(0), piNormalizado(0.0), mejorArea(0), menorMunicion(900), actualMunicionGastada(0), pixeles(nullptr), pixelesSoloCircunferencia(nullptr)
    {
        this->circulos = new Circulo[M];
        this->mejorEleccion = new bool[M]{};
        this->circulosElegidos = new bool[M]{};
    }
    int **crearMatriz(int altura, int anchura)
    {
        int **m = new int *[altura];
        for (int i = 0; i < altura; i++)
        {
            m[i] = new int[anchura];
            for (int j = 0; j < anchura; j++)
                m[i][j] = 0;
        }
        return m;
    }

    ~Imagen()
    {
        liberarMatriz(pixeles, alto);
        liberarMatriz(pixelesSoloCircunferencia, alto);

        if (circulos != nullptr)
            delete[] circulos;
        if (mejorEleccion != nullptr)
            delete[] mejorEleccion;
        if (circulosElegidos != nullptr)
            delete[] circulosElegidos;
    }
    void liberarMatriz(int **m, int altura)
    {
        if (m == nullptr)
            return;
        for (int i = 0; i < altura; i++)
            delete[] m[i];
        delete[] m;
    }

    // #core

    void procesarPGM(int N)
    {
        string nombrePGM = "objectives" + to_string(N) + ".pgm";
        ifstream PGM(nombrePGM.c_str());
        if (!PGM.is_open())
        {
            cout << "Archivo .pgm no encontrado." << endl;
            exit(-1);
        }

        string formatoPGM;
        PGM >> formatoPGM;
        if (formatoPGM != "P2")
        {
            cout << "Error. El archivo .pgm abierto no es P2";
            exit(-1);
        }
        else
        {
            PGM >> this->ancho;
            PGM >> this->alto;
            PGM >> this->colorMaximo;

            pixeles = crearMatriz(alto, ancho);
            pixelesSoloCircunferencia = crearMatriz(alto, ancho);

            for (int i = 0; i < this->alto; i++)
            {
                for (int j = 0; j < this->ancho; j++)
                {
                    PGM >> this->pixeles[i][j];
                }
            }
        }
        PGM.close();
    }
    void detectarCirculos(int M)
    {
        int circuloID = 1;

        for (int i = 0; i < this->alto; i++)
        {
            for (int j = 0; j < this->ancho; j++)
            {
                if (this->pixeles[i][j] > 0)
                {
                    int colorNuevo = this->pixeles[i][j];
                    if (colorVisitado[colorNuevo] == 0)
                    {
                        if (circuloID > M)
                        {
                            return;
                        }

                        colorVisitado[colorNuevo] = circuloID;
                        this->circulos[circuloID - 1].id = circuloID;
                        this->circulos[circuloID - 1].color = colorNuevo;
                        circuloID++;
                    }

                    int circuloActual = colorVisitado[colorNuevo];
                    int index = circuloActual - 1;

                    if (verificarBorde(this->pixeles, i, j))
                    {
                        definirIntervalos(this->circulos[index], i, j);

                        this->circulos[index].circunferencia++;
                        this->pixelesSoloCircunferencia[i][j] = this->pixeles[i][j];
                    }
                }
            }
        }
    }
    void analizarCirculos(int &M)
    {
        int colorCentro = this->colorMaximo;
        int cuantosM_menos = 0;
        for (int i = 0; i < M; i++)
        {
            if (circulos[i].id == -1)
            {
                cuantosM_menos++;
                continue;
            }

            this->circulos[i].encontrarCentro();
            int indexX = circulos[i].centro.x;
            int indexY = circulos[i].centro.y;
            this->pixelesSoloCircunferencia[indexY][indexX] = colorCentro;

            this->circulos[i].encontrarDiametro();
            this->circulos[i].encontrarRadio();
            this->circulos[i].encontrarPi();
        }
        M = M - cuantosM_menos;
        ordenarPorOscuridad(M);
    }
    void crearHerramientas(int M)
    {
        normalizarPi(M);
        normalizarArea(M);
    }
    void encontrarAreaNormalizadaTotal(int circuloActual, int M, int T, int areaAproximada)
    {
        if (mejorArea == T && actualMunicionGastada >= menorMunicion)
            return;

        int sumaAreaRestante = 0;
        for (int i = circuloActual; i < M; i++)
            sumaAreaRestante += circulos[i].AND;
        if (areaAproximada + sumaAreaRestante < mejorArea)
            return;

        if (circuloActual == M)
        {
            if ((areaAproximada >= mejorArea) && (areaAproximada <= T))
            {
                if (areaAproximada > mejorArea)
                {
                    mejorArea = areaAproximada;
                    menorMunicion = actualMunicionGastada;
                    for (int i = 0; i < M; i++)
                    {
                        mejorEleccion[i] = circulosElegidos[i];
                    }
                    return;
                }
                else if (areaAproximada == mejorArea && areaAproximada > 0)
                {
                    if (actualMunicionGastada < menorMunicion)
                    {
                        menorMunicion = actualMunicionGastada;
                        for (int i = 0; i < M; i++)
                            mejorEleccion[i] = circulosElegidos[i];
                        return;
                    }
                }
            }
            return;
        }

        if (areaAproximada + circulos[circuloActual].AND <= T)
        {
            circulosElegidos[circuloActual] = true;
            actualMunicionGastada++;
            encontrarAreaNormalizadaTotal(circuloActual + 1, M, T, areaAproximada + circulos[circuloActual].AND);
            circulosElegidos[circuloActual] = false;
            actualMunicionGastada--;
        }
        encontrarAreaNormalizadaTotal(circuloActual + 1, M, T, areaAproximada);
    }
    void mostrarSolucion(int M)
    {

        cout << endl;
        for (int i = 0; i < M; i++)
        {
            cout << circulos[i].AND << "u" << endl;
        }
        cout << endl;

        bool primerCirculo = true;
        for (int i = 0; i < M; i++)
        {
            if (mejorEleccion[i])
            {
                if (!primerCirculo)
                    cout << ", ";
                cout << "Circulo" << circulos[i].id;
                primerCirculo = false;
            }
        }

        cout << endl;
    }
    void generarPGM()
    {
        ofstream archivo("sites.pgm");
        if (archivo.is_open())
        {
            archivo << "P2" << endl;
            archivo << this->ancho << " " << this->alto << endl;
            archivo << this->colorMaximo << endl;

            for (int i = 0; i < this->alto; i++)
            {
                for (int j = 0; j < this->ancho; j++)
                {
                    archivo << this->pixelesSoloCircunferencia[i][j] << " ";
                }
                archivo << endl;
            }
            archivo.close();
        }
        else
        {
            cout << "Error al crear sites.pgm" << endl;
        }
    }

    // #tools
    void intercambiarCirculos(int i, int j)
    {
        Circulo temp = circulos[i];
        circulos[i] = circulos[j];
        circulos[j] = temp;
        int idTemp = circulos[j].id;
        circulos[j].id = circulos[i].id;
        circulos[i].id = idTemp;
    }
    void normalizarPi(int M)
    {
        for (int i = 0; i < M; i++)
        {
            this->piNormalizado += this->circulos[i].pi;
        }
        this->piNormalizado /= (float)M;
    }
    void normalizarArea(int M)
    {
        float areaNormalizada = 0;
        for (int i = 0; i < M; i++)
        {
            areaNormalizada = this->piNormalizado * (this->circulos[i].radio * this->circulos[i].radio);
            this->circulos[i].AND = ceil(areaNormalizada);
        }
    }
    void definirIntervalos(Circulo &circulo, int i, int j)
    {
        if (circulo.ordenadas.inicio == -1)
            circulo.ordenadas.inicio = i;

        if (i > circulo.ordenadas.final)
            circulo.ordenadas.final = i;

        if (circulo.abscisas.inicio == -1 || j < circulo.abscisas.inicio)
            circulo.abscisas.inicio = j;

        if (j > circulo.abscisas.final)
            circulo.abscisas.final = j;
    }
    void ordenarPorOscuridad(int M)
    {
        for (int i = 0; i < M - 1; i++)
        {
            for (int j = 0; j < M - i - 1; j++)
            {
                if (this->circulos[j].color > this->circulos[j + 1].color)
                {
                    intercambiarCirculos(j, j + 1);
                }
            }
        }
    }
    bool verificarBorde(int **mapa, int i, int j)
    {
        int miColor = mapa[i][j];
        if (i == 0 || j == 0 || i == this->alto - 1 || j == this->ancho - 1)
            return true;

        if (
            mapa[i][j - 1] != miColor || // pa verificar izquierda
            mapa[i - 1][j] != miColor || // pa verificar arriba
            mapa[i][j + 1] != miColor || // pa verificar derecha
            mapa[i + 1][j] != miColor)
        { // pa verificar abjo
            return true;
        }

        return false;
    }
};

// #main
int main()
{
    int N, M, T;

    cin >> N >> M >> T;

    Imagen imagen(M);
    imagen.procesarPGM(N);
    imagen.detectarCirculos(M);
    imagen.analizarCirculos(M);
    imagen.crearHerramientas(M);
    imagen.encontrarAreaNormalizadaTotal(0, M, T, 0);
    imagen.mostrarSolucion(M);
    imagen.generarPGM();
    
    return 0;
}
