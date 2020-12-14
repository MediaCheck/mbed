### Testování targetu BYZANCE_IODAG3E pomocí GreenTea

## Funkce: 

MBED OS má připraveno sadu zhruba 60 testů, které slouží k otestování chodu OS na Hardwaru.
Testovací tool se skládá z tří důležitých částí

* Mbed Cli 
  github: https://github.com/ARMmbed/mbed-cli
* Mbed GreenTea
  github: https://github.com/ARMmbed/greentea
* Mbed HTRun
  github: https://github.com/ARMmbed/htrun

#### MBED CLI
je nejvyšší vrstva testovacího toolu a slouží k hlavnímu ovládání testů, buildí testovací 
binárky a spouští MBED GreenTea

#### MBED GREENTEA
Stará se o průběh testu, ověřuje dostupnost Hardwaru a všech potřebných skriptů a souborů. 
Po ověření spustí mbedhtrun.

#### MBED HTRun
nejnižší vrstva toolu přímo komunikuje se zařízením, flashuje binárku,resetuje HW, obsluhuje 
sériovou linku. 

### Instalace

Všechny čsti toolu se dají stáhnout pomocí "pip install"

```Python
pip install mbed-cli
pip install mbed-greentea
```
mbed HTRun by měl být distribuován spoečně s mbed greentea. Pokud něco nebude fungovat na 
githubu mají podrovný instalační návod.

Je potřeba mít také stažený **greentea client**. Bežne se stahuje společně s mbedem, nicméně 
z toho našeho se mazal, protože byl zbytečný .. Dá se stáhnout z githubu: https://github.com/ARMmbed/mbed-os/tree/master/features/frameworks
a vložit do **mbed/features/frameworks**

## Konfigurace pro target BYZANCE_IODAG3E

Testovací tool mbedu není připraven na buildění customizovaných targetů a je potřeba pár věcí předělat. 
V první fázi je potřeba nastavit, aby bylo zařízení detekováno jako správný target. HTRun k **flashování 
využívá DRAG & DROP**, takže je nutné použít NUCLEO ST-PLACEHOLDER nebo náš DAP LINK. Pokud je zařízení připojeno 
přes ST-PLACEHOLDER, je nutné **úplně odpojit NUCLEO**... To znamená odpojit jumper U5V. Pokud se tak neučiní, 
Nucleo bude do testování mluvit a testovací tool nerozpozná výsledky testů. Poté je nutné vlézt do složky Mbedu 
(Tam kde se nachází mbed_settings.py) a zapnout konsolový emulátor. V emulátoru zavolat příkaz 

```
mbed detect
```

Tento příkaz se podívá na deskriptory všech připojených zařízení, a zjistí co jsou zač a která z nich podporují MBED OS.
Pokud je připojený IODA přes ST PlaceHolder nebo i přes DAP LINK a tabulka bude prázdná, znamená to, že zařízení přímo 
nepodporuje mbed OS. Zařízení jsou detekována podle kódu připojeného chipu. A proto je potřeba změnit aby kod ST-PLACEHOLDERU
a DAP LINKU mířil na target BYZANCE_IODAG3E. To je potřeba změnit v knihovně, která se stáhla společně s knhovnamy HT-RUN. 
a nachází se ve složce pythonu

```
C:/Python27/Lib/site-packages/mbed_lstools/lstools_base.py
```

zde se nachází seznam 'manufacture_ids'. V tomto seznamu je potřeba najít ST-PLACEHOLDER, 
nebo pro DAPLINK target ARCH_PRO a jejich název přepsat za target BYZANCE_IODAG3E. 

Dále je potřeba nakonfigurovat target BYZANCE_IODAG3E v souboru mbed/targets/targets.json. je potřeba přidat řádek 

```
"release_versions": ["2","5"],
```

a zároveň musí mít 

```
"supported_toolchains": ["GCC_ARM","ARM","IAR"],
```

I přes to že používáme většinou toolchain GCC_ARM, ostatní tam musí být, aby mbed uznal že zařízení podporuje mbed 5. 
Pokud je správně nakonfigurováno a zařízení připojené, příkaz "mbed detect" by měl vrátit tabulku ve které bude 
target BYZANCE_IODAG3E se všemi vyplněnými políčky "Supported".

## Testování 

### Zapojení

Zařízení je potřeba připojit programovacím konektorem k programátoru, aby mbedHTRun mohl flashnou testovací binárku a 
zároveň je potřeba propojit sériovou linku IODY a programátoru(PLACEHOLDERU nebo DAPLINKu) po které bude Ioda vracet výsledky testů.
U Nuclea **POZOR** aby bylo opravdu **úplně** vypnuté, jinak bude odpovídat na dotazy mbedu a bude kazit testování.

### Flashování

Pokud se k flashování používá DAP_LINK, IODA už v sobě může mít BOOLOADER, protože DAP LINK dokáže zařídít posun na adresu 8010000 
a není nic potřeba měnit. Pokud se využívá ST-PLACEOLDER, tak u toho posun na adresu 801 nejde udělat jinak, než přes ST-LINK (což HTRun neumí).
Je ale možnost flashovat bez bootloaderu přímo na adresu 8000000. Testování to neovlivní, ale je potřeba přepsat všechny 
makra právě na adresu 8000000.. jak v **target.json** u **"macros"**, tak také u linkeru 

```
mbed\targets\TARGET_STM\TARGET_STM32F4\TARGET_STM32F437xI\TARGET_BYZANCE_IODAG3E\STM32F437II_offset.ld
```

řádek

```
FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 2048k - 64k. 
```

### Průběh testu

Primárně se využívá k testování příkaz provedený stejně jako "mbed detect" ve složce mbedu

```
mbed test -m BYZANCE_IODAG3E -t GCC_ARM
```

tento příkaz najde všechny testy psané v c++ ve složce TESTS a sbuildí je spoečně s operačním systémem do složky BUILD.
Pokud build proběhne vpořádku, rovnou spustí greentea a pokusí se testovat. 

Ke specifikaci, co se bude testovat, slouží soubor test_spec.json ve složce 
mbed/BUILD/tests/BYZANCE_IODAG3E/GCC_ARM. Binárky vybuilděných testu se zde pak nachází ve složce TESTS.

Pokud je zařízení připojeno, HTRun vybere první testovací binárku a flashne jí do zažízení. Poté zařízení resetuje
a vyšle po seriové lince komunikační preambuli (pokud k příkazu přidáte přepínač -v zapne se verbose režim a komunikaci
lze podrobně videt v konzoli). Flashnuté zařízení na preambuli odpoví a řekne co bude testovat.. poté se na zařízení provede 
test a jeho výsledky jsou zaslány po seriové lince zpět do HTRun. ten je vyhodnotí a začne flashovat další testovací binárku .. 

## Vlastní test
Testy se dělí na skupiny. Můžeme definovat vlastní skupinu testů, například 

```
TESTS/byzance/
```

a v té psát vlastní testy. Pokud chceme provést pouze jeden konkrétní test, lze provést příkaz

```
mbed test -m BYZANCE_IODAG3E -t GCC_ARM -n "tests-byzance-mujprvnitest"
```

Ten se musí nacházet tedy ve složce "TESTS/byzance/mujprvnitest/" a musí být pojmenovaný main.cpp. 

Pokud chceme provést skupinu testů, tak to je potřeba specifikovat přímo v souboru test_spec.json.

### Struktura vlastního testu

Každý test musí mít takovouto strukturu:

```cpp
#include "mbed.h"
#include "greentea-client/test_env.h"
#include "unity.h"
#include "utest.h"
#include "rtos.h"

using namespace utest::v1;

// A test that returns successfully is considered successful
void test_success() {
    TEST_ASSERT(true);
}

// Tests that assert are considered failing
void test_failure() {
    TEST_ASSERT(false);
}

utest::v1::status_t test_setup(const size_t number_of_cases) {
    // Setup Greentea using a reasonable timeout in seconds
    GREENTEA_SETUP(40, "default_auto");
    return verbose_test_setup_handler(number_of_cases);
}

// Test cases
Case cases[] = {
    Case("Testing success test", test_success),
    Case("Testing failure test", test_failure),
};

Specification specification(test_setup, cases);

// Entry point into the tests
int main() {
    return !Harness::run(specification);
}
```

Program provede postupně všechny funkce specifikované v Case cases[]{}.


