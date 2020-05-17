Projekt ICP 2019/2020 -- Simulátor liniek hromadnej dopravy

Autori:
Peter Koprda (xkoprd00)
Adam Múdry (xmudry01)

Popis programu:
Program po spustení požiada používateľa o otvorenie súboru s dátami (JSON súbor obsahujúci dáta o linkách, autobusoch, uliciach a zastávkach). Po vybratí súboru sa vytvorí interaktívna simulácia hromadnej dopravy.

Linky sú definované zastávkami, trasu cez zadané ulice medzi nimi vytvorí A* (pathfinding) algoritmus, ktorú autobusy na danej linke kopírujú. Po príjazde do konečnej zastávky autobus čaká 3 sekundy a vyrazí naspäť do začiatočnej zastávky. Na každej linke je 10 autobusov vychádzajúcich po 1 každých 10 sekúnd.

Používateľ môže spomaliť premávku na vybranej ulici alebo ju zablokovať/odblokovať. Ak linka nemá trasu cez zastávku na zablokovanej ulici, pokúsi sa ju obísť. Ak jej trasa vedie cez zastávku an zablokovanej ulici, alebo nevie nájsť inú trasu do cieľa, zostane stáť na mieste. Po posunutí myši na ulicu na mape sa zobrazí jej názov.

Po vybratí linky/autobusu na danej linke je danú linku možné editovať -- vytvoriť jej úplne novú trasu klikaním na zastávky a následným uložením po kliknutí na tlačidlo "Save" (ak používateľ vyberie iba 1 zastávku, zmena sa neuloží). Pôvodné trasy autobusov ide obnoviť kliknutím na tlačidlo "Reset All".

Simuláciu je možné zrýchliť, pozastaviť, spustiť od začiatku, pretočiť dorpedu/dozadu o 1 sekundu, priblížiť.

Po označení položky na mape/jej vybraní v bočnom menu sa v lište nad scénou vypíšu informácie o danej položke -- napr. po kliknutí na autobus sa vypíše: číslo autobusu, číslo linky, na akej ulici sa nachádza, začiatočná zastávka, konečná zastávka, posledná naštívená zastávka, zastávka kam autobus smeruje.

Makefile:
make         -- preloží program
make run     -- preloží program a spustí ho
make clean   -- vymaže vygenerované súbory
make pack    -- vytvorí archív pre odovzdanie
make doxygen -- vytvorí dokumentáciu

Preložený program sa nachádza v zložke src/.

Odovzdávané súbory:
README.txt
Makefile
doc/Doxyfile
doc/README.md
examples/city.json
examples/square_town.json
src/main.cpp
src/mainwindow.cpp
src/pathfinding.cpp
src/scene.cpp
src/mainwindow.h
src/pathfinding.h
src/scene.h
src/datastructures.h
src/icp.pro
