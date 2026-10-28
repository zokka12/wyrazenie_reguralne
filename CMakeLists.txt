cmake_minimum_required(VERSION 3.10)
project(RegexMatcher CXX)

# Wymagamy standardu C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Definicja pliku wykonywalnego i połączonych plików źródłowych
add_executable(RegexMatcher_Podstawy main.cpp podstawy_regrex.cpp)
#add_executable(RegexMatcher_Zaawansowany main.cpp rozsz_regrex.cpp)

# Dodanie flag ostrzeżeń kompilatora dla naszego celu (targetu)
target_compile_options(RegexMatcher_Podstawy PRIVATE -Wall -Wextra)
#target_compile_options(RegexMatcher_Zaawansowany PRIVATE -Wall -Wextra)