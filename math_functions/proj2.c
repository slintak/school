/******************************************************************************
 *
 * Projekt cislo 2 do predmetu BIZP -- Iteracni vypocty
 *
 * == Zadání úlohy ==
 * Napište program, který pomocí iteračních algoritmů zpracuje libovolně
 * dlouhou posloupnost číselných hodnot typu double, které budou v textové
 * podobě zapsány na standardní vstup a budou odděleny libovolně dlouhou
 * posloupností bílých znaků.
 *
 * Výstupem programu bude číselná posloupnost výsledků. Používejte
 * konstanty (makra) NAN a INFINITY pro ošetření výjimečných případů
 * tak, aby to dávalo smysl.
 * 
 * Autor: 	Vlastimil Slintak <xslint01@stud.feec.vutbr.cz>
 * AutorID:	121035@FEKT
 * Datum: 	Thu, 23 Dec 2010 11:58:14 +0200
 * Verze: 	1.0
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#define NDEBUG
#include <assert.h>

/**
 * Matematicke konstanty
 */
const double IZP_E = 2.7182818284590452354;        // e
const double IZP_PI = 3.14159265358979323846;      // pi
const double IZP_2PI = 6.28318530717958647692;     // 2*pi
const double IZP_PI_2 = 1.57079632679489661923;    // pi/2
const double IZP_PI_4 = 0.78539816339744830962;    // pi/4
/**
 * arctanh(1+DBL_EPSILON) = 0.5 ln(2/DBL_EPSILON)
 *
 * Tato konstanta nam urci nejmensi mozny argument funkce
 * tanh(), ktery jiz vrati cislo 1.0
 * Od tohoto cisla se jiz nemusi volat tanh() ale jen vratit
 * jednicka.
 */
double IZP_TANH_MAX;

/**
 * Akce programu
 */
enum actions {
	A_HELP,		// Vytiskne napovedu
	A_TANH,		// Vypocita tanh zadaneho cisla
	A_LOGAX,	// Vypocita logaritmus cisla o zadanem zakladu
	A_WAM,		// Aritmeticky vazeny prumer
	A_WQM,		// Kvadraticky vazeny prumer
	A_ERR		// Chybove hlaseni
};

/**
 * Chybove kody, ktere mohou vracet funkce
 */
enum errors {
	E_OK,		// Vsechno probehlo OK
	E_CL,		// Chyba parametru prikazoveho radku
	E_UNKW,		// Neznama chyba
	E_MEM,		// Chyba pameti
	E_INT,		// Uzivatel zadal spatne cislo
	E_STR,
	E_EOF,		// Konec vstupniho retezce
};

/**
 * Ke kazdemu chybovemu kodu je prirazen retezec popisujici chybu.
 */
const char *ERROR_MSG[] = {
	"Vse probehlo bez chyby.\n",
	"Spatne zadane vstupni parametry!\n",
	"Nastala neznama chyba!\n",
	"Nemohu alokovat pamet. Pravdepodobne neni dostatek pameti?\n",
	"Chybne zadane vstupni parametry! Nelze prevest na cislo\n",
	"Chyba vstupniho retezce\n",
};

/**
 * Napoveda k programu
 */
int HELP_MSG_SIZE = 11;
const char *HELP_MSG[] = {
	"Program je soucasti projektu cislo 2 do predmetu BIZP:",
	"\tIteracni vypocty\n",
	"\n",
	"Autor: Vlastimil Slintak, 121035@FEKT\n",
	"\n",
	"Parametry programu jsou:\n",
	" -h\t\t\tVytiskne tuto napovedu a skonci\n",
	" --tanh sigdig N\tVypocita tanh s presnosti na 'sigdig' desetinnych mist.\n",
	" --logax sigdig a\tVypocita logaritmus o zakladu 'a'.\n",
	" --wam\t\t\tPrubezne pocita vazeny aritm. prumer.\n",
	" --wqm\t\t\tPrubezne pocita vazeny kvadr. prumer.\n"
};

/**
 * Do teto struktury se ukladaji parametry a akce prikazove radky
 */
typedef struct parameters {
	unsigned int sigdig;	// Pocet desetinnych mist vysledku
	double a;		// Zaklad logaritmu.
	int action;		// Akce, ktera se ma vykonat
	int error;		// Chybovy kod programu (pouze pokud nastala chyba)
} t_parameters;

/**
 * Tento datovy typ se pouziva pro prubezne
 * ukladani vysledku u statistickych funkci
 */
typedef struct _s_mean {
	double sum;		// Soucet soucinu vah a hodnot
	double count;		// Soucet vah
} s_mean;

/**
 * Funkce obaluje volani strtoul() a spravne osetruje
 * chybove stavy, ktere muze tato funkce vracet.
 *
 * @param str Retezec s celym cislem
 * @param num Zde bude ulozen vysledek
 */
bool safe_str_to_uint(char *str, unsigned int *num) {
	if(str == NULL)
		return false;

	errno = 0;
	int number = strtol(str, NULL, 10);
	if(errno != 0) {
		return false;
	}

	if(number < 0)
		return false;

	*(num) = (unsigned int)number;
	return true;
}

/**
 * Funkce obaluje volani strtod(). Jediny rozdil
 * s funkci safe_str_to_uint() je prevod double
 * misto int
 */
bool safe_str_to_dbl(char *str, double *num) {
	if(str == NULL)
		return false;

	errno = 0;
	*num = strtod(str, NULL);
	if(errno != 0) {
		return false;
	}
	return true;
}

/**
 * Funkce zpracuje vstupni parametry prikazoveho radku a vrati akci, kterou
 * uzivatel vyzadal.
 * Velmi jednoducha implementace.
 */
t_parameters get_params(int argc, char *argv[]) {
	t_parameters params = {0, 0, A_ERR, E_CL};

	//
	// -h --> napoveda
	if(argc == 2) {
		if(strcmp("-h", argv[1]) == 0) {
			params.action = A_HELP;
			params.error = E_OK;
		} else
	//
	// --wam --> Vazeny aritmeticky prumer
		if(strcmp("--wam", argv[1]) == 0) {
			params.action = A_WAM;
			params.error = E_OK;
		} else
	//
	// --wqm --> Vazeny kvadratocky prumer
		if(strcmp("--wqm", argv[1]) == 0) {
			params.action = A_WQM;
			params.error = E_OK;
		}
	} else if(argc == 3 || argc == 4) {
	//
	// --tanh --> Vypocet tangens hyperbolicky
		if(strcmp("--tanh", argv[1]) == 0) {
			params.action = A_TANH;
			params.error = E_OK;
			// Prevod retezce na cislo
			if(!safe_str_to_uint(argv[2], &(params.sigdig))) {
				params.action = A_ERR;
				params.error = E_INT;
			}
	//
	// --logax --> Vypocet logaritmu
		} else if(strcmp("--logax", argv[1]) == 0) {
			params.action = A_LOGAX;
			params.error = E_OK;
			// Prevod retezce na cislo
			if(!safe_str_to_uint(argv[2], &(params.sigdig)) ||
			   !safe_str_to_dbl(argv[3], &(params.a))) {
				params.action = A_ERR;
				params.error = E_INT;
			}
		}
	}
	return params;
}


/**
 * Spocita exponencialni fci cisel mensich jak jedna a vetsich jak nula.
 * Vypocet je realizovan Taylorovou radou v jednom pruchodu.
 * 	e^x = sum( x^n / n! )
 */
double approx_by_dbl_exp(double x, double epsilon) {
	assert(x < 1.0);
	assert(x >= 0.0);

	double y = 1;
	double frac = 1;
	double x_pow = x;
	unsigned int n = 32;
	for(unsigned int i = 2; i < n; i++) {
		double last = x_pow * frac;
		y += last;
		frac /= i;
		x_pow *= x;
		// Pokud jsme jiz vyhoveli presnosti, vrati vysledek
		if(last < epsilon)
			break;
	}
	return y;
}

/**
 * Vypocet exponencialni funkce z celociselne casti parametru. Celociselna
 * cast argumentu se nasobi po mocninach dvojky. Napriklad:
 * 	e^(5.7) => e^(101.111) ve dvojkove soustave
 * 	
 * Celociselna cast je v tomto pripade '101', tedy:
 * 	e^5 => e^(101) => e^4 * e^1 => e^(100) * e^(001)
 *
 * K vysledku se dostaneme dvojim nasobenim, rekurzivne se zanorime trikrat.
 * Casova narocnost tohoto algoritmu je tedy 'log(x)'.
 *
 *   Prvni parametr je argument funkce 'x'. Po navratu z funkce obsahuje
 * desetinny zbytek, mensi jak jedna.
 *   'exp_e' musi byt pri volani fce roven '1.0', rekurzivnim volanim
 * pak obsahuje exp_e^exp_2n
 *   'exp_2n' je pri volani fce roven '1', rekurzivnim volanim pak
 * obsahuje 2^n, kde 'n' je hloubka zanoreni.
 *
 * @param x Ukazatel na argument funkce. Po navratu obsahuje desetinny zbytek
 * @param exp_e Pri volani fce je roven 1.0, pak e^2^n
 * @param exp_2n Pri volani fce je roven jedne, pak 2^n
 */
double approx_by_int_exp(double *x, double exp_e, unsigned int exp_2n) {
	assert(*x >= 1);

	exp_e *= (exp_e == 1) ? IZP_E : exp_e;
	if(*x < exp_2n)
		return 1;

	double y = approx_by_int_exp(x, exp_e, exp_2n*2);
	if(*x >= exp_2n) {
		*x -= exp_2n;
		return y * exp_e;
	} else
		return y;
}

/**
 * Vypocita exponencialni funkci cisla 'x' s presnosti vetsi
 * nez 'epsilon'
 */
double approx_exp(double x, double epsilon) {
	double approx_by_int = 1.0;
	// Nejdrive spocitame celociselnou cast
	if(x >= 1.0)
		approx_by_int = approx_by_int_exp(&x, 1.0, 1);
	// Pote desetinnou a oba vysledky vynasobime
	return approx_by_dbl_exp(x, epsilon) * approx_by_int;
}

/*
double approx_sinh(double x, double epsilon) {
	double y = 0;
	double frac = 1;
	double x_pow = x;
	for(unsigned int i = 1; i < 32; i++) {
		frac /= i;
		if(i%2 != 0) {
			y += x_pow * frac;
			x_pow *= x * x;
	
		}
	}
	return y;
}

double approx_cosh(double x, double epsilon) {
	double y = 1.0;
	double frac = 1;
	double x_pow = 1;
	for(unsigned int i = 1; i < 32; i++) {
		frac /= i;
		x_pow *= x;
		if(i%2 == 0) {
			y += x_pow * frac;
	
		}
	}
	return y;
}
*/

/**
 * Spocita hyperbolicky tangens
 * K tomu vyuziva exponencialni funkce
 */
double approx_tanh(double x, double epsilon) {
	// tanh(0) = 0
	if(x == 0.0)
		return 0.0;
	
	// NAN nebudeme pocitat, vratime NAN
	if(isnan(x))
		return NAN;

	// Pokud uzivatel zada vetsi cislo, nez ktere jsme schoppni
	// vyjarit v double, vratime jednicku
	if(x >= IZP_TANH_MAX)
		return 1.0;

	// Funkce tanh() je licha, vlevo od nuly zaporna.
	double operator = 1.0;
	if(x < 0)
		operator = -1.0;


	double e2x = approx_exp(2*x*operator, epsilon);
	return operator * (e2x - 1) / (e2x + 1);
}

double exp_a_2n(double a, unsigned int n) {
	double y = a;
	for (unsigned int i = 0; i < n; ++i)
		y = y * y;
	return y;
}

/**
 * Spocitame logaritmus pro celociselnou cast.
 *
 * @param a Zaklad logaritmu
 * @param x Argument logaritmu
 * @param residual_x Zbytek po vypoctu.
 */
unsigned int integer_log_a1_x1(double a, double x, double *residual_x) {
	assert(a > 1);
	assert(x > 0);

	unsigned int y = 0;
	int n = CHAR_BIT * sizeof(unsigned int); // Pocet bitu unsigned int je
						// pocet iteraci, ktere musime provest.
	// Projdeme cislo 'x' zleva do prava. Tedy od MSB k LSB.
	for (; n >= 0; --n) {
		y <<= 1;
		// Pocitat pokazde exp_a_2n() je zdlouhave a zbytecne,
		// vhodnejsi by bylo predem si vypocitat tyto hodnoty
		// do pole, ze ktereho pak jiz jenom cist.
		double x_with_exp_bit_taken = x / exp_a_2n(a, n);
		if (x_with_exp_bit_taken >= 1) {	// Je MSB exponentu nastaven?
			x = x_with_exp_bit_taken;	// Vynuluj ho.
			y |= 1;	// Pridej bit k vysledku
		}
	}
	assert(x < a);  // V tomto okamziku by argument mel byt mensi jak zaklad.
			// Pokud tomu tak neni, je chyba v algoritmu.

	*residual_x = x; // Uzivateli vratime zbytek argumentu.
	return y;
}

/**
 * Pocita logaritmus cisla 'x' o zakladu 'a'. Ocekavame,
 * ze cislo 'x' bude v intervalu (1,a). Zde se pocita
 * desetinna cast logaritmu
 */
double fractional_log_a1_x1(double a, double x) {
	assert(a > 1);
	assert(x <= a);
	assert(x >= 1);

	double exp_n = 1;	// exp_n = n-ta mocnina cisla 2.
	unsigned int n = 32;	// Pocet bitu, ktere budeme prochazet
	double y = 0;
	for (unsigned int i = 0; i < n; i++) {
		y *= 2;		// Posun bity v exponentu doleva
		x = x * x;	
		if (x >= a) {	// Je MSB exponentu nastaven?
			y += 1;	// Uloz bit do vysledku...
			x /= a;	// ...a vynuluj MSB exponentu.
		}
		exp_n *= 2;
	}
	
	return y / exp_n;
}

/**
 * Spocita logaritmus cisla 'x' o zakladu 'a'.
 * Nejdrive se spocita celociselna cast logaritmu, pote
 * desetinna a vysledky se sectou.
 */
double log_a1_x1(double a, double x) {
	double residual_x;
	unsigned int integer_part = integer_log_a1_x1(a, x, &residual_x);
	double fractional_part = fractional_log_a1_x1(a, residual_x);
	return integer_part + fractional_part;
}

/**
 * Pokud je 'x' v intervalu (0,1), volame logaritmus s jeho
 * prevracenou hodnotou a vracime vysledek s opacnym znamenkem.
 */
double log_a1_x(double a, double x) {
	return x < 1 ? -log_a1_x1(a, 1 / x) : log_a1_x1(a, x);
}

double approx_log_a(double a, double x) {
	// Pokud uzivatel zadal nekonecno,
	// vracime nekonecno
	if(isinf(a) || isinf(x))
		return INFINITY;

	// Pro 'a', 'x' <= 0 nebudeme pocitat
	// a = 1 take nema smysl
	if (a <= 0 || x <= 0 || a == 1) {
		return NAN;
	} else {
		// Pokud je zaklad a < 1, zavolame funkci log s
		// jeho prevracenou hodnotou a vratime zaporne
		// cislo. Viz dokumentace.
		if(a < 1)
			return -log_a1_x(1 / a, x);
		else
			return log_a1_x(a, x);
	}
}

/**
 * Spocita obecny logaritmus, nebo hyperbolicky tanges.
 *
 * Pokud je 'logax' nastaven jako true, pocita se logaritmus,
 * jinak hyperbolicky tangens.
 */
int eval_math(bool logax, t_parameters params) {
	// Nejdrive si prevedeme pocet desetinnych
	// mist na cislo
	// 5 -> 0.00001
	double epsilon = 1;
	for(unsigned int i = 0; i < params.sigdig; i++)
		epsilon /= 10;

	double x;
	int scanf_return;
	// Postupne nacitej vstupni data a vysledky
	// tiskni na stdout
	while(1) {
		scanf_return = scanf("%lf", &x);
		if(scanf_return == EOF)
			break;
		else if(scanf_return < 1)
			return E_STR;

		// Rozhodneme, kterou funkci budeme volat
		double result = NAN;
		if(logax == true)
			result = approx_log_a(params.a, x);
		else
			result = approx_tanh(x, epsilon);
		// Vytiskneme vysledek
		printf("%.10e\n", result);
	}
	return E_OK;
}

s_mean *alloc_mean(void) {
	s_mean *mean = malloc(sizeof *mean);
	if(mean == NULL)
		return NULL;

	// Vynulujeme hodnoty
	mean->sum = 0;
	mean->count = 0;

	return mean;
}

/**
 * Pocita vazeny aritmeticky prumer. Funkce muze byt
 * volana vicekrat, prubezne vysledky si uklada do
 * struktury s_mean.
 */
double count_wam(s_mean *wm, double x_n, double w_n) {
	// Pokud je ukazatel na strukturu s prubeznymi
	// vysledky NULL, vracime NAN.
	if(wm == NULL)
		return NAN;

	// Pokud uzivatel zadal zapornou vahu prvku,
	// vratime NAN.
	if(w_n <= 0.0)
		return NAN;

	wm->sum += x_n * w_n;
	wm->count += w_n;

	return wm->sum / wm->count;
}

/**
 * Pocita vazeny kvadraticky prumer. Funkce muze byt
 * volana vicekrat, prubezne vysledky si uklada do
 * struktury s_mean.
 */
double count_wqm(s_mean *wm, double x_n, double w_n) {
	// Pokud je ukazatel na strukturu s prubeznymi
	// vysledky NULL, vracime NAN.
	if(wm == NULL)
		return NAN;

	// Pokud uzivatel zadal zapornou vahu prvku,
	// vratime NAN.
	if(w_n <= 0.0)
		return NAN;

	wm->sum += x_n * x_n * w_n;
	wm->count += w_n;

	return sqrt(wm->sum / wm->count);
}

/**
 * Prubezne nacita cisla od uzivatele
 * a na stdout vypisuje vysledky.
 * 
 * Pokud je 'wam' True, pocita se vazeny aritmeticky
 * prumer, jinak vazeny kvadraticky prumer.
 */
int eval_wm(bool wam) {
	s_mean *wm = alloc_mean();
	if(wm == NULL)
		return E_MEM;

	while(1) {
		double value, weight;
		if(scanf("%lf", &value) < 1)
			break;
		if(scanf("%lf", &weight) < 1) {
			printf("nan\n");
			free(wm);
			return E_STR;
		}

		double result = NAN;
		if(wam)
			result = count_wam(wm, value, weight);
		else
			result = count_wqm(wm, value, weight);

		printf("%.10e\n", result);
	}

	free(wm);
	return E_OK;
}

int main(int argc, char *argv[]) {
	// Uplne na zacatku si vypocitame potrebne konstanty
	IZP_TANH_MAX = 0.5 * approx_log_a(IZP_E, 2/DBL_EPSILON);

	// Nacteme parametry dane uzivatelem
	t_parameters params = get_params(argc, argv);
	int return_code = E_UNKW;

	// Provedeme zadanou akci
	switch(params.action) {
		case A_ERR:
			fprintf(stderr, "%s", ERROR_MSG[params.error]);
			return 1;
		case A_HELP:
			for(int i = 0; i < HELP_MSG_SIZE; i++)
				fprintf(stdout, "%s", HELP_MSG[i]);
			return 0;
		case A_TANH:
			return_code = eval_math(false, params);
			break;
		case A_LOGAX:
			return_code = eval_math(true, params);
			break;
		case A_WAM:
			return_code = eval_wm(true);
			break;
		case A_WQM:
			return_code = eval_wm(false);
			break;
		default:
			fprintf(stderr, "%s", ERROR_MSG[E_UNKW]);
			return 1;

	}
	
	// V pripade neuspechu vratime chybove hlaseni
	if(return_code != E_OK) {
		fprintf(stderr, "%s", ERROR_MSG[return_code]);
		return 1;
	} else
		return 0;
}
