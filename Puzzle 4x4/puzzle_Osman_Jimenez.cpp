/**
* Autor : Osman David Jimenez Gutierrez
* Lenguaje : C++11
*/
#include <bits/stdc++.h>

#define ii pair<int, int>

using namespace std;

struct State{
    vector<int> board;
    int estimated, real, wx, wy, total;

    State(int estimated, vector<int> board, int real, int wx, int wy) : estimated(estimated), board(board), real(real), wx(wx), wy(wy) {
        total = estimated + real;
    }

    bool operator < (const State &x) const {
        return total > x.total;
    }
};

        /// R D L U
int DX[] = {0,1,0,-1};
int DY[] = {1,0,-1,0};

int N = 4, ans;

unordered_set< long long > seen;
vector<int> board, solved;
vector<long long> factorial(17, 0);
vector<ii> posCorrect(16);

/**
    Funcion que se encarga de imprimir el tablero, setw se usa para fijar K posiciones cada vez
    que imprima una posicion del tablero
**/
void printBoard(vector<int> &v) {
    int K = 2;
    for(int i = 0; i < N*N; i += 4) {
        for(int j = 0; j < N * K + N; j++) cout << "-"; cout << endl;
        for(int j = i; j < i + N; j++)
            cout << setw(K) << v[j] << "|";
        cout << endl;
    }
    for(int i = 0; i < N * K + N; i++) cout << "-"; cout << endl;
}

/** Funcion que se encarga de cambiar dos posiciones (a, b) del tablero (v) **/
inline void doSwap(ii a, ii b, vector<int> &v) {
    swap(v[a.first*4 + a.second], v[b.first*4 + b.second]);
}

/**
    Funcion que se encarga de hacer un tablero random, para que el random sea mejor
    se revisa que el movimiento que vaya a hacer, no sea el mismo que devolverse, esto
    es necesario ya que si no se hace, la complejidad del tablero seria minima, sobretodo
    para los casos de 25 movimientos random, facilmente podria arrojar un tablero con solucion
    de 8 movimientos
**/
ii shuffleBoard(int times) {

    int wx = N - 1, wy = N - 1;
    int px = -1, py = -1;
    for(int i = 1; i <= times; i++) {
        int d = rand() % 4, nx = wx + DX[d], ny = wy + DY[d];
        if(nx >= 0 && ny >= 0 && nx < N && ny < N && (px != nx || py != ny)) {
            doSwap(ii(nx, ny), ii(wx, wy), board);
            px = wx; py = wy;
            wx = nx; wy = ny;
        }
        else {
            i--;
        }
    }
    return ii(wx, wy);
}

/** Verifica si el tablero (v) es una solucion del puzzle 16 **/
bool isSolved(vector<int> &v) {
    for(int i = 0; i < N*N; i++)
        if(v[i] != solved[i])
            return false;
    return true;
}

/**
    Funcion que se encarga de inicializar el tablero, y el tablero solucion. Tambien
    precalcula el factorial hasta 16, y inicializa las posiciones (x, y) de cada valor
    del puzzle 16 (0,1,2....,14,15)
**/
void init() {
    for(int i = 0; i < N*N - 1; i++) board.push_back(i + 1);
    board.push_back(0);
    solved = board;

    factorial[0] = 1;
    for(int i = 1; i <= 16; i++)
        factorial[i] = (long long)i * factorial[i - 1];

    for(int i = 0; i < N*N; i += 4)
        for(int j = i; j < i + N; j++)
            posCorrect[ solved[j] ] = ii(i/4, j%4);

}

/**
    Funcion que genera el numero de permutacion que representa el tablero, la permutacion
    que se forma con los numeros (0,1,2.....,14,15), esto para memorizar el estado en el
    algoritmo de IDS
**/
long long hashCode(vector<int> &v) {
    int used = 0;
    long long ret = 0;
    for(int i = 0, j = v.size() - 1; i < v.size(); i++, j--) {
        int low = (v[i]) - __builtin_popcount(((1 << v[i]) - 1) & used);
        ret += low * factorial[j];
        used |= 1 << v[i];
    }
    return ret;
}


/**
    Funcion que simula el algoritmo IDS, memoriza el estado del tablero mediante un
    unordered_set
**/
bool IDS(vector<int> &u, int wx, int wy, int depth, int limit) {
    ans++;
    if(isSolved(u)) return true;
    if(depth == limit) return false;
    long long hs = hashCode(u);
    seen.insert(hs);

    for(int i = 0; i < 4; i++) {
        int nx = wx + DX[i];
        int ny = wy + DY[i];
        if(nx >= 0 && ny >= 0 && nx < N && ny < N) {
            doSwap(ii(nx, ny), ii(wx, wy), u);
            if(!seen.count(hashCode(u)) && IDS(u, nx, ny, depth + 1, limit))
                return true;
            doSwap(ii(nx, ny), ii(wx, wy), u);
        }
    }

    seen.erase(hs);
    return false;
}

/**
    Funcion que se encarga de llamar a la funcion IDS con los diferentes limites
    de profundidad
**/
int solveWithIDS(ii p) {
    ans = 0;
    for(int i = 0; ; i++) {
        seen.clear();
        if(IDS(board, p.first, p.second, 0, i)) break;
    }
    return ans;
}

/** Funcion que calcula el valor de la heuristica del niño rabon para un tablero (v) **/
int getCheaterChild(vector<int> v) {
    int ret = 0;
    for(int i = 0; i < N*N; i++) {
        if( v[i] == 0 ) continue;
        ret += (v[i] != solved[i]);
    }
    return ret;
}

/** Funcion que calcula el valor de la heuristica del niño rabon para un tablero (v) **/
int getManhattan(vector<int> v) {
    int ret = 0;
    for(int i = 0; i < N*N; i += 4)
        for(int j = i; j < i + N; j++) {
            int val = v[j];
            if( val == 0 ) continue;
            ret += abs(i/4 - posCorrect[val].first) + abs(j%4 - posCorrect[val].second);
        }
    return ret;
}

/**
    Funcion que simula el algoritmo A*, recibe una pareja de enteros (p) que es la posicion
    del espacio en blanco. Si flag es verdadero, usa la heuristica de manhattan, sino, la
    heuristica del niño rabon
**/
int solveWithAStar(ii p, bool flag) {
    ans = 1;
    priority_queue< State, vector<State> > pq;
    if(flag) pq.push(State(getManhattan(board), board, 0, p.first, p.second));
    else     pq.push(State(getCheaterChild(board), board, 0, p.first, p.second));
    while(!pq.empty()) {
        State cur = pq.top(); pq.pop();

        if(isSolved(cur.board)) break;

        for(int i = 0; i < 4; i++) {
            int nx = cur.wx + DX[i];
            int ny = cur.wy + DY[i];
            if(nx >= 0 && ny >= 0 && nx < N && ny < N) {
                ans++;
                doSwap(ii(nx, ny), ii(cur.wx, cur.wy), cur.board);
                if(flag) pq.push(State(getManhattan(cur.board), cur.board, cur.real + 1, nx, ny));
                else     pq.push(State(getCheaterChild(cur.board), cur.board, cur.real + 1, nx, ny));
                doSwap(ii(nx, ny), ii(cur.wx, cur.wy), cur.board);
            }
        }

    }

    return ans;
}

/** Funcion que calcula la desviacion estandar **/
double getDesvest(double x, vector<double> &v, double sz) {
    double var = 0.0;
    for(int i = 0; i < v.size(); i++) var += ((double)v[i] - x) * ((double)v[i] - x);
    return sqrt(var/sz);

}

/** Funcion que calcula e imprime las estadisticas del experimiento **/
void getStatistics(vector<double> &v, double sz) {
    double media = 0, mediana = 1.0*(v[sz/2] + v[sz/2 - 1]) / 2.0;
    for(int i = 0; i < v.size(); i++) media += v[i];
    media /= 30.0;
    cout << "\tMedia            = " << media << endl;
    cout << "\tMediana          = " << mediana << endl;
    cout << "\tDesvest(Media)   = " << getDesvest(media, v, sz) << endl;
    cout << "\tDesvest(Mediana) = " << getDesvest(mediana, v, sz) << endl;

}

/** Funcion que simula los experimentos **/
void printStatistics() {
    for(int moves = 10; moves <= 25; moves += 5) {
        cout << "************** Para " << moves << " movimientos random *******************" << endl;
        vector<double> resultsIDS, resultsMan, resultsCheat;

        for(int i = 0; i < 30; i++) {
            board = solved;
            ii p = shuffleBoard(moves);
            resultsMan.push_back(solveWithAStar(p, true));

            resultsCheat.push_back(solveWithAStar(p, false));

            resultsIDS.push_back(solveWithIDS(p));
        }

        cout << endl;
        sort(resultsMan.begin(), resultsMan.end());
        sort(resultsCheat.begin(), resultsCheat.end());
        sort(resultsIDS.begin(), resultsIDS.end());

        cout << "A* (Manhattan):" << endl;
        getStatistics(resultsMan, 30);
        cout << "A* (CheaterChild):" << endl;
        getStatistics(resultsCheat, 30);
        cout << "Iterative Deepening Search:" << endl;
        getStatistics(resultsIDS, 30);
        cout << endl;


    }
}

int main() {
    ios_base::sync_with_stdio(0);
    srand(time(NULL));
    /**Descomentar para ver la salida en un archivo de texto**/
    //freopen("output.txt", "w", stdout);

    init();
    printStatistics();

    /**Este es un caso de prueba donde la respuesta es 21 de profundidad**/
    /*board = {5,1,2,3,9,8,6,4,13,10,11,15,14,12,0,7};
    p = ii(3, 2);/**/
    /**Este es un caso de prueba donde la respuesta es 25 de profundidad**/
    /*board = {2,5,7,6,0,3,1,4,9,10,12,8,13,14,11,15};
    p = ii(1,0);/**/
    /*cout << solveWithAStar(p, true) << endl;
    cout << solveWithAStar(p, false) << endl;
    cout << solveWithIDS(p) << endl;/**/

}
