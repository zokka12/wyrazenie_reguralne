#include <iostream>
#include <string>
#include "regrex.h"

using namespace std;

int main() {
    cout << "Sprawdzanie wyrazen regularnych (wpisz 'exit' aby zakonczyc)\n\n";

    while (true) {
        string pattern, text;

        cout << "Podaj wyrazenie regularne: ";
        if (!getline(std::cin, pattern) || pattern == "exit") break;

        try {
            regex re(pattern);

            std::cout << "Podaj tekst: ";
            if (!getline(std::cin, text)) break;
            
            bool czy_pasuje = re.matches(text);
            string wynik = czy_pasuje ? "PASUJE " : "NIE PASUJE";

            cout << "-> " << wynik << "\n\n";
            
        } catch (const std::exception& e) {
            cout << "Niepoprawne wyrazenie : " << e.what() << "\n\n";
        }
    }
    
    cout << "Zakonczono testy.\n";
    return 0;
}
