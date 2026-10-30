#include "regrex.h"
#include <iostream>
#include <ostream>
#include <stack>
#include <stdexcept>
#include <algorithm>

using namespace std;
// słówko static nadaje łączenie wewnętrzne co ogranicza widoczność tej funkci do pliku źródłowego
static bool czy_liczba(const string& s)
{
    if (s.empty()) return false;
    for (char c : s) if (c < '0' || c > '9') return false;
    return true; 
}

string regex::zrob_preprocessing(string re) const {
   
    string wynik = ""; // tworzymy pusty schowek (string) do niego będziemy powoli dopisywać nasz nowy przerobiony wzorzec
    bool w_kwasnych = false; // tworzymy flagę do sprawdzania czy jesteśmy w nawiasach kwadratowych

    // analiza wyrażenia znak po znaku
    for (size_t i = 0; i < re.length(); i++) { // re.length() określa długość napisu
        char c = re[i]; 
        // char – typ danych służący do przechowywania pojedynczego znaku
        // c – nazwa zmiennej typu char, do której przypisujemy aktualnie badany znak
        // re[i] – wyciąga z napisu re konkretny pojedynczy znak znajdujący się na pozycji o numerze i

        if (c == '[') {
            w_kwasnych = true;
            wynik += '(';
        } 
        else if (c == ']' && w_kwasnych) {
            w_kwasnych = false;
            wynik += ')';
        } 
        else if (w_kwasnych) {
            wynik += c; // Dopisujemy aktualną literę do wyniku

            if (i + 1 < re.length() && re[i + 1] != ']') {
                wynik += '|'; 
            }
            // sprawdzamy czy kolejny znak to nie ], jeśli nie to dodajemy | między znakami
        } 
        else {
            wynik += c; // przepisuje znak bez zmian
        }
    }

    // Przypisujemy przetworzony ciąg z [ab] -> (a|b) z powrotem do re
    re = wynik;


size_t pozycja_klamry_otwieracej= re.find('{');
    while (pozycja_klamry_otwieracej != string::npos)
    {
        size_t pozycja_klamry_zamykajacej = re.find('}', pozycja_klamry_otwieracej);
        // (od_którego_indeksu, ile_znaków)
        string parametry = re.substr(pozycja_klamry_otwieracej+ 1, pozycja_klamry_zamykajacej - pozycja_klamry_otwieracej - 1);

        // szukamy początku elementu przed klamrą // start element do powtórzenia np a 
        size_t start = pozycja_klamry_otwieracej - 1;
        if (re[start] == ')')
        {
            int licznik = 1;
            while (licznik > 0)
            {
                start--;
                if (re[start] == ')') licznik++;
                if (re[start] == '(') licznik--;
            }
        }
        string element = re.substr(start, pozycja_klamry_otwieracej - start);

        int min = stoi(parametry);
        string nowy_tekst = "";
        for (int i = 0; i < min; i++)
            nowy_tekst += element;

        size_t przecinek = parametry.find(',');
        if (przecinek != string::npos)
        {
            if (przecinek + 1 == parametry.size())      
            {
                nowy_tekst += element + "*";
            }
            else                                       
            {
                int max = stoi(parametry.substr(przecinek + 1));
                for (int i = min; i < max; i++)
                    nowy_tekst += element + "?";
            }
        }

        re.replace(start, pozycja_klamry_zamykajacej - start + 1, nowy_tekst);
        pozycja_klamry_otwieracej = re.find('{');
    }

    return "(" + re + ")"; 
}

//{} ->  możemy zamienić na ? , trzeba brać pod uwage nawiasy 
// (a|b)  równoważne [ab]
regex::regex(const string& raw_re) {
    

    int zbalansowanie = 0;
    for (char c : raw_re) {
        if (c == '(') zbalansowanie++;
        else if (c == ')') {
            zbalansowanie--;
            if (zbalansowanie < 0) throw invalid_argument("Niezbalansowane nawiasy");
        }
    }
    if (zbalansowanie != 0) throw invalid_argument("Niezbalansowane nawiasy");
    
    wzorzec = zrob_preprocessing(raw_re);
    dlugosc_wzorca = wzorzec.length();
    // +1 tworzy ostateczny stan sukcesu, do którego automat wskakuje po przeczytaniu ostatniej litery
    darmowe_przejscia.resize(dlugosc_wzorca + 1);
    
    // stos analityczny lifo
    // służy do parsowania gramatyki. Ponieważ nawiasy mogą być zagnieżdżone, zapamiętuje historię otwieranych nawiasów i alternaty
    stack<int> stos_operatorow;

    for (int i = 0; i < dlugosc_wzorca; i++) 
    {
        int lewy_nawias = i;
        char znak = wzorzec[i]; 

        if (znak == '*' || znak == '+' || znak == '?') {
        // Jeśli operator jest na samej pierwszej pozycji (i == 0)
        // lub bezpośrednio po znaku, którego nie da się powtórzyć
        if (i == 0 || wzorzec[i - 1] == '(' || wzorzec[i - 1] == '|') {
            throw invalid_argument("Niepoprawne wyrazenie");
            }
        }

        // obsługa nawiasów i alternatywy za pomocą stosu
            if (znak == '(') 
            {
            // odkładamy na stos, żeby wiedzieć, gdzie zaczęła się ta konkretna grupa
            stos_operatorow.push(i);

            // do vectora sa dodawane przejscia
            darmowe_przejscia[i].push_back(i + 1); // nawias otwierający jest "przezroczysty" dla automatu
            } 
            else if (znak == '|') {
            // odkładamy indeks alternatywy, zzeka on na stosie aż do ')'
            stos_operatorow.push(i);

            } 
            else if (znak == ')') {
            vector<int> indeksy_alternatyw;
            
            // sciągamy ze stosu wszystkie alternatywy '|' należące do tej grupy
            while (!stos_operatorow.empty() && wzorzec[stos_operatorow.top()] == '|')
                {
                indeksy_alternatyw.push_back(stos_operatorow.top());
                stos_operatorow.pop();
                }
            
            //pod spodem musi znajdować się nawias otwierający '('. Jeśli go nie ma - błąd.
            if (stos_operatorow.empty() || wzorzec[stos_operatorow.top()] != '(') {
                throw invalid_argument("Niezbalansowane nawiasy");
                }
            
            //pobieramy indeks nawiasu '(' i usuwamy go ze stosu
            lewy_nawias = stos_operatorow.top(); 
            stos_operatorow.pop();
            darmowe_przejscia[i].push_back(i + 1);

            // lączenie ścieżek alternatywy w graf
            // nawias '(' dostaje ścieżkę prosto do znaku po '|'
            // znak '|' dostaje ścieżkę prosto do nawiasu ')' 
            for (int alt_idx : indeksy_alternatyw) {
                darmowe_przejscia[lewy_nawias].push_back(alt_idx + 1); 
                darmowe_przejscia[alt_idx].push_back(i);               
                }
            }

        // dlaczego przy tym kodzie 

            // kwatyfikatory +,*, ? 
        if (i + 1 < dlugosc_wzorca) {
            char nastepny_znak = wzorzec[i + 1];

            if (nastepny_znak == '*') {
                // * - może wystąpić 0 lub więcej razy.
                // skok w przód  oraz skok w tył 
                darmowe_przejscia[lewy_nawias].push_back(i + 1); 
                darmowe_przejscia[i + 1].push_back(lewy_nawias); 
                darmowe_przejscia[i + 1].push_back(i + 2);
           
            } 
            else if (nastepny_znak == '+') {
                // + - Musi wystąpić co najmniej 1 raz.
                // tylko skok w tył , brak skoku w przód wymusza przeczytanie choć raz.
                darmowe_przejscia[i + 1].push_back(lewy_nawias); 
                darmowe_przejscia[i+1].push_back(i + 2);
          
            } 
            else if (nastepny_znak == '?') {
                // ? - występuje 0 lub 1 raz.
                // tylko skok w przód, brak powtarzania.
                darmowe_przejscia[lewy_nawias].push_back(i + 1); 
                darmowe_przejscia[i+1].push_back(i + 2);
            
            }
        }
    }
}

bool regex::matches(const string& tekst) const {
    //automat zajmuje wszystkie startowe pozycje naraz przez DFS
    vector<bool> aktywne_stany(dlugosc_wzorca + 1, false);
    oznacz_osiagalne_stany(0, aktywne_stany);

    //Główna pętla wczytująca tekst litera po literze
    for (size_t i = 0; i < tekst.length(); i++) {
        vector<int> dopasowane_stany; // żeby do konsoli z zrzucać vector za pomocą pętli for 

        // tutaj wypisywać na konsoli zawarttoąś struktury aktywne_stany, żeby zobaczyć, które stany są dopasowane po wczytaniu każdej litery
        cout << "Aktualnie aktywne stany po wczytaniu litery " << tekst[i] << ": ";
        for (int j = 0; j <= dlugosc_wzorca; j++) {
            if (aktywne_stany[j]) {
                cout << j << " ";
            }
        }
        cout << endl;

        
        // v to poprostu indeks, iterujemy po indeksie, dług wzorca to wszystkie ineksy + stan sukcesu wiec niech sie wtedy zatszyma i tak po wszystkich indeksach

        // Sprawdzamy, które z aktualnie aktywnych stanów NFA pasują do wczytanej litery
        for (int v = 0; v < dlugosc_wzorca; v++) {
            char r = wzorzec[v];
            bool czy_operator = (r == '*' || r == '+' || r == '?' || r == '(' || r == ')' || r == '|');
            
            if (aktywne_stany[v] && !czy_operator) {
            //if (aktywne_stany[v]) {
                // Jeśli znak w wyrażeniu regularnym pasuje do wczytanej litery lub jest kropką ('.'), dodajemy stan v+1 do dopasowanych stanów
                // tak jak robiliśmy na tablicy i wypisywaliśmy {0,1,2,3,4,5,6,7,8,9} i tak dalej
                if (r == tekst[i] || r == '.') {
                    dopasowane_stany.push_back(v + 1);
                }
            }
        }

        // tutaj wypisywać na konsoli zawarttoąś struktury dopasowane_stany, żeby zobaczyć, które stany są dopasowane po wczytaniu każdej litery
        cout << "Dopasowane stany po wczytaniu litery " << tekst[i] << ": ";
        for (int stan : dopasowane_stany) {
            cout << stan << " ";
        }
        cout << endl;

        // Po wczytaniu litery, sprawdzamy, które stany są osiągalne z dopasowanych stanów
        fill(aktywne_stany.begin(), aktywne_stany.end(), false);
        for (int v : dopasowane_stany) {
            if (!aktywne_stany[v]) oznacz_osiagalne_stany(v, aktywne_stany);
        }

        //
    }
    
    return aktywne_stany[dlugosc_wzorca];
}

void regex::oznacz_osiagalne_stany(int start_stan, vector<bool>& odwiedzone_stany) const {
    // stos przeszukiwania w głąb w wersji iteracyjnej
    // napędza algorytm przechodzenia po grafie. 
    // stos pilnuje, w które odnogi (darmowe przejścia epsilon) trzeba jeszcze wejść.
    stack<int> stos;
    stos.push(start_stan);
    odwiedzone_stany[start_stan] = true;

    while (!stos.empty()) 
    {
        int aktualny = stos.top();
        stos.pop();
        for (int nastepny : darmowe_przejscia[aktualny]) {
            // zabezpieczenie przed nieskończonym zapętleniem 
            if (!odwiedzone_stany[nastepny]) {
                odwiedzone_stany[nastepny] = true;
                stos.push(nastepny); // wrzucamy nowo odkryty stan na szczyt stosu
            }
        }
    }
}