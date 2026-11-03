#include "regrex.h"
#include <iostream>
#include <ostream>
#include <stack>
#include <stdexcept>
#include <algorithm>
#include <cctype>

using namespace std;
// słówko static nadaje łączenie wewnętrzne co ogranicza widoczność tej funkci do pliku źródłowego
// mamy {a1,}; {,9}
static bool czy_liczba(const string& s) {
    if (s.empty()) return false;
    // po kolei każdy znak c z napisu s
    for (char c : s) 
        if (c < '0' || c > '9') return false;
    return true; 
}

static int safe_stoi (const string& s) {
    int wartosc;
    try{
        wartosc = stoi(s);
    }
    catch (const exception&) { 
        throw invalid_argument("Niepoprawna liczba w klamrze" ); 
    }
    if (wartosc > 10000) {
        throw invalid_argument("Liczba w klamrze przekracza bezpieczny limit powtorzen (max 10000)" );
    }
    return wartosc;
}
//tabulacja 07 przejście na dwubajtowe kody, 
static bool czy_pasuje_do_klasy(char wzorzec_znak, char tekst_znak) {
    if (wzorzec_znak == '\x01') return isdigit(tekst_znak); // \d - cyfra
    else if (wzorzec_znak == '\x02') return isalnum(tekst_znak) || tekst_znak == '_'; // \w - znak słowny
    else if (wzorzec_znak == '\x03') return isspace(tekst_znak); // \s - znak biały
    else if (wzorzec_znak == '\x04') return tekst_znak == '.';
    else if (wzorzec_znak == '\x05') return tekst_znak == '*';
    else if (wzorzec_znak == '\x06') return tekst_znak == '+';
    else if (wzorzec_znak == '\x07') return tekst_znak == '?';
    else if (wzorzec_znak == '\x08') return tekst_znak == '(';
    else if (wzorzec_znak == '\x09') return tekst_znak == ')';
    else if (wzorzec_znak == '\x0A') return tekst_znak == '|';
    else if (wzorzec_znak == '\x0B') return tekst_znak == '[';
    else if (wzorzec_znak == '\x0C') return tekst_znak == ']';
    else if (wzorzec_znak == '\x0D') return tekst_znak == '{';
    else if (wzorzec_znak == '\x0E') return tekst_znak == '}';
    else if (wzorzec_znak == '\x0F') return tekst_znak == '\\';
    else if (wzorzec_znak == '.') return true; // dowolny znak
    return wzorzec_znak == tekst_znak;
}

string regex::zrob_preprocessing(string re) const {
    // Sprawdzanie poprawności nawiasów w wyrażeniu regularnym tutaj zamiast w regrex zanim wrzucimy w nawiasy
    int bilans = 0;
    bool w_kwasnych_walidacja = false;
    for (size_t i = 0; i < re.length(); i++) {
        if (re[i] == '\\') { i++; continue; } // pomija sprawdzanie znaków ucieczki
        if (re[i] == '[') w_kwasnych_walidacja = true;
        else if (re[i] == ']') w_kwasnych_walidacja = false;
    
        if (!w_kwasnych_walidacja) {
            if (re[i] == '(') bilans++;
            else if (re[i] == ')') {
                bilans--;
                if (bilans < 0) throw invalid_argument("Niezbalansowane nawiasy");
            }
        }
    }
    if (bilans != 0) throw invalid_argument("Niezbalansowane nawiasy");    
    string po_ucieczkach = ""; // tworzymy pusty string, do którego będziemy dopisywać kolejne znaki z re po przetworzeniu ucieczek

    for (size_t i = 0; i < re.length(); i++) {
        if (re[i] == '\\' ) {
            if ( i + 1 < re.length()){
            char nast = re[i + 1]; // sprawdzamy znak po slashu
            if (nast == 'd') po_ucieczkach += '\x01'; // zamieniamy \d na znak specjalny \x01
            else if (nast == 'w') po_ucieczkach += '\x02'; // zamieniamy \w na znak specjalny \x02
            else if (nast == 's') po_ucieczkach += '\x03'; // zamieniamy \s na znak specjalny \x03
            else if (nast == '.') po_ucieczkach += '\x04';
            else if (nast == '*') po_ucieczkach += '\x05';
            else if (nast == '+') po_ucieczkach += '\x06';
            else if (nast == '?') po_ucieczkach += '\x07';
            else if (nast == '(') po_ucieczkach += '\x08';
            else if (nast == ')') po_ucieczkach += '\x09';
            else if (nast == '|') po_ucieczkach += '\x0A';
            else if (nast == '[') po_ucieczkach += '\x0B';
            else if (nast == ']') po_ucieczkach += '\x0C';
            else if (nast == '{') po_ucieczkach += '\x0D';
            else if (nast == '}') po_ucieczkach += '\x0E';
            else if (nast == '\\') po_ucieczkach += '\x0F';
            else po_ucieczkach += nast; // zachowanie dla np. \. lub \*
            i++;
            }else {
        throw invalid_argument("Samotny ukosnik na koncu wyrazenia");
    }
        // pomijamy znak po slashu
        } else {
            po_ucieczkach += re[i];
        }
    }
    re = po_ucieczkach; // Podmiana łańcucha przed jego dalszą rozbudową
   
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
    if (w_kwasnych) {
        throw invalid_argument("Brak zamykającego nawiasu kwadratowego ']'");
    }

    // Przypisujemy przetworzony ciąg z [ab] -> (a|b) z powrotem do re
    re = wynik;


size_t pozycja_klamry_otwieracej= re.find('{');
    while (pozycja_klamry_otwieracej != string::npos)
    {
        size_t pozycja_klamry_zamykajacej = re.find('}', pozycja_klamry_otwieracej);
        // (od_którego_indeksu, ile_znaków)

        // jeśli nie znaleziono zamykającej klamry, rzucamy wyjątek
        if (pozycja_klamry_zamykajacej == string::npos) 
        {
            throw invalid_argument("Brak zamykajacej klamry '}'");
        }

        if (pozycja_klamry_zamykajacej + 1 < re.length()) {
            char nxt = re[pozycja_klamry_zamykajacej + 1];
            if (nxt == '{' || nxt == '*' || nxt == '+' || nxt == '?') {
                throw invalid_argument("Niedozwolona sekwencja kwantyfikatorow po klamrze ");
            }
        }
        // jeśli klamra otwierająca jest na początku wyrażenia, rzucamy wyjątek
        if (pozycja_klamry_otwieracej == 0) 
        {
            throw invalid_argument("Klamra na poczatku wyrazenia - brak elementu do powtorzenia");
        }

         // szukamy początku elementu przed klamrą // start element do powtórzenia np a 
        size_t start = pozycja_klamry_otwieracej - 1;
        char poprzedzajacy = re[start];
        if (poprzedzajacy == '*' || poprzedzajacy == '+' || poprzedzajacy == '?' || 
            poprzedzajacy == '(' || poprzedzajacy == '|' ) 
        {
            throw invalid_argument("Brak poprawnego elementu do powtorzenia przed klamra");
        }


        string parametry = re.substr(pozycja_klamry_otwieracej+ 1, pozycja_klamry_zamykajacej - pozycja_klamry_otwieracej - 1);

        if (parametry.empty()) 
        {
            throw invalid_argument("Klamra nie może byc pusta");
        }

        // szukamy początku elementu przed klamrą // start element do powtórzenia np a 


        if (re[start] == ')')
        {
            int licznik = 1;
            while (licznik > 0)
            {
                if (start == 0) 
                {
                    throw invalid_argument("Niezbalansowane nawiasy");
                }
                start--;
                if (re[start] == ')') licznik++;
                if (re[start] == '(') licznik--;
            }
        }
        string element = re.substr(start, pozycja_klamry_otwieracej - start);

        size_t przecinek = parametry.find(',');
        int min = 0;
        // zabezpiecza to by w klamrach nie było liter
        if (przecinek == string::npos)
            {
                if(!czy_liczba(parametry)) 
                    throw invalid_argument("W klamrze jest litera");
                
                min = safe_stoi(parametry);
            }
        else {
            string przed = parametry.substr(0, przecinek);
            string po = parametry.substr(przecinek + 1);

            if (!przed.empty() && !czy_liczba(przed))
                throw invalid_argument("zle minimum w klamrze");
            if (!po.empty() && !czy_liczba(po))
                throw invalid_argument("zle maksimum w klamrze");
            if (!przed.empty()){
                min = safe_stoi(przed);
            }
        }
        //int min = stoi(parametry);
        string nowy_tekst = "";
        for (int i = 0; i < min; i++)
            nowy_tekst += element;

        if (przecinek != string::npos) {
            if (przecinek + 1 == parametry.size()) {
                nowy_tekst += element + "*";
            }
            else                                       
            {
                int max = safe_stoi(parametry.substr(przecinek + 1));
                // sprawdzamy czy min nie jest większe od max
                if (min > max) 
                {
                    throw invalid_argument("Nielogiczny przedzial w klamrze: min > max");
                }
                for (int i = min; i < max; i++)
                    nowy_tekst += element + "?";
            }
        }
        //gdy wynik rozwinięcia jest pusty
        if(nowy_tekst.empty()){
            nowy_tekst = "()"; // dodajemy pusty nawias, aby zachować poprawną składnię
        }
        //(od_którego_indeksu, ile_znaków_wyciąć, czym_zastąpić)
        re.replace(start, pozycja_klamry_zamykajacej - start + 1, nowy_tekst);

        pozycja_klamry_otwieracej = re.find('{');
    }

    return "(" + re + ")"; 
}


//{} ->  możemy zamienić na ? , trzeba brać pod uwage nawiasy 
// (a|b)  równoważne [ab]
regex::regex(const string& raw_re) {
        // \s, \d,\w - jak budować maszyne, 
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

        if (znak == '|') {
            if (i == 0 || wzorzec[i-1] == '(' || wzorzec[i-1] == '|') throw invalid_argument("Pusta alternatywa");
        }
        if (znak == ')') {
            if (wzorzec[i-1] == '(' || wzorzec[i-1] == '|') throw invalid_argument("Pusta grupa wewnatrz nawiasow");
        }

        if (znak == '*' || znak == '+' || znak == '?') {
        // Jeśli operator jest na samej pierwszej pozycji (i == 0)
        // lub bezpośrednio po znaku, którego nie da się powtórzyć
        if (i == 0 || wzorzec[i - 1] == '(' || wzorzec[i - 1] == '|' || wzorzec[i - 1] == '*' || wzorzec[i - 1] == '+' || wzorzec[i - 1] == '?') {
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
    // sprawdzamy, czy stos operatorów jest pusty po przetworzeniu całego wyrażenia czyli w sytuacji, gdy wszystkie nawiasy i alternatywy zostały poprawnie zamknięte
    if (!stos_operatorow.empty()) {
        throw invalid_argument("Niezbalansowane nawiasy");
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
        //cout << "Aktualnie aktywne stany po wczytaniu litery " << tekst[i] << ": ";
        //for (int j = 0; j <= dlugosc_wzorca; j++) {
            //if (aktywne_stany[j]) {
                //cout << j << " ";
            //}
        //}
        //cout << endl;

        
        // v to poprostu indeks, iterujemy po indeksie, dług wzorca to wszystkie ineksy + stan sukcesu wiec niech sie wtedy zatszyma i tak po wszystkich indeksach

        // Sprawdzamy, które z aktualnie aktywnych stanów NFA pasują do wczytanej litery
        for (int v = 0; v < dlugosc_wzorca; v++) {
            char r = wzorzec[v];
            bool czy_operator = (r == '*' || r == '+' || r == '?' || r == '(' || r == ')' || r == '|');
            
            if (aktywne_stany[v] && !czy_operator) {
            //if (aktywne_stany[v]) {
                // Jeśli znak w wyrażeniu regularnym pasuje do wczytanej litery lub jest kropką ('.'), dodajemy stan v+1 do dopasowanych stanów
                // tak jak robiliśmy na tablicy i wypisywaliśmy {0,1,2,3,4,5,6,7,8,9} i tak dalej
                if (czy_pasuje_do_klasy(r, tekst[i])) {
                    dopasowane_stany.push_back(v + 1);
                }
            }
        }

        // tutaj wypisywać na konsoli zawarttoąś struktury dopasowane_stany, żeby zobaczyć, które stany są dopasowane po wczytaniu każdej litery
        //cout << "Dopasowane stany po wczytaniu litery " << tekst[i] << ": ";
        //for (int stan : dopasowane_stany) {
            //cout << stan << " ";
        //}
        //cout << endl;

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