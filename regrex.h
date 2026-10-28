#pragma once

#include <string>
#include <vector>

class regex {
private:
    std::string wzorzec;
    int dlugosc_wzorca;

    // vector odpowiadający za przechowywanie NFA, każdy indeks przechowuje listę, do której można przejść za pomocą pustego przejścia 
    std::vector<std::vector<int>> darmowe_przejscia;
    
    // tablice do obsługi klas dopasowania
    //std::vector<bool> czy_jest_ucieczka; 
    //std::vector<std::string> klasa_dopasowania;
    //std::vector<bool> jest_negacja;

    // przeszukiwanie w głąb w wersji iteracyjnej
    void oznacz_osiagalne_stany(int start_stan, std::vector<bool>& odwiedzone_stany) const;

    // funkcja do wstępnego przetwarzania wyrażenia regularnego
    std::string zrob_preprocessing(std::string re) const;

public:
// konstruktor, który przyjmuje wyrażenie regularne w postaci stringa
// explicit oznacza, że nie można użyć tego konstruktora do niejawnej konwersji typu std::string na regex
    explicit regex(const std::string& raw_re);

    // funkcja sprawdzająca, czy dany tekst pasuje do wyrażenia regularnego
    bool matches(const std::string& tekst) const;
};