/******************************************************************************
 *
 * Projekt cislo 3 do predmetu BIZP -- Maticove operace
 *
 * Autor: 	Vlastimil Slintak <xslint01@stud.feec.vutbr.cz>
 * AutorID:	121035@FEKT
 * Datum: 	Thu, 30 Dec 2010 23:58:14 +0200
 * Verze: 	1.0
 ******************************************************************************/
#ifndef __PROJ3_H__
#define __PROJ3_H__

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/**
 * Typy se kterymi se pracuje mohou byt:
 *  - matice
 *  - vektor
 *  - vektor matic (3D matice)
 */
#define VECTOR		1
#define	MATRIX		2
#define	MATRICES	3

/**
 * Pocet prvku v poli v hlavicce
 * matice. Kazda matice muze byt definovana
 * rozmery X, Y, Z.
 */
#define MATRIX_HEADER	3
#define MATRIX_X	1
#define MATRIX_Y	2
#define MATRIX_Z	0

/**
 * Vrati maximalni rozmer X, Y nebo Z matice M
 */
#define MATRIX_GET_X(M)			((M)->header->dims[MATRIX_X])
#define MATRIX_GET_Y(M)			((M)->header->dims[MATRIX_Y])
#define MATRIX_GET_Z(M)			((M)->header->dims[MATRIX_Z])
/**
 * Ze zadanych souradnic matice vrati jeji polohu v poli.
 */
#define MATRIX_COORDINATES(M, X, Y, Z)	( (X) + (Y)*MATRIX_GET_X(M) + (Z)*MATRIX_GET_X(M)*MATRIX_GET_Y(M))

/**
 * Vraci true pokud se jedna o dany typ, jinak false.
 */
#define IS_VECTOR(T)			( ((T)->header->type == VECTOR) ? true : false )
#define IS_MATRIX(T)			( ((T)->header->type == MATRIX) ? true : false )
#define IS_MATRICES(T)			( ((T)->header->type == MATRICES) ? true : false )

/**
 * Struktura do ktere se ukladaji vsechny mozne
 * vstupni parametry programu.
 */
typedef struct {
	char *name;			// Jmeno parametru
	int args_no;			// Pocet argumentu, ktere parametr vyzaduje
	int (*function)(char **);	// Funkce, ktera se bude volat
	char *help;			// Napoveda - strucny popis funkce
} parameter_s;

/**
 * Tato skupina funkci obaluje matematicke funkce
 * uzivatelskym rozhranim a vypisuje vysledky.
 */
int call_help(char ** argv);
int call_test(char ** argv);
int call_vadd(char **argv);
int call_vscal(char **argv);
int call_mmult(char **argv);
int call_mexpr(char **argv);
int call_bubbles(char **argv);

/**
 * Staticke definice moznych argumentu programu
 */
static int parameters_no = 8;
static parameter_s parameters[] = {
	{"-h",		0, call_help, "Vytiskne tuto napovedu"},
	{"--test",	1, call_test, "Zkontroluje jestli je zadany soubor validni."},
	{"--vadd",	2, call_vadd, "Secte dva vektory A + B a vytiskne vysledek."},
	{"--vscal",	2, call_vscal,"Vypocita skalarni soucin dvou vektoru a vytiskne vysledek."},
	{"--mmult",	2, call_mmult,"Vypocita soucin dvou matic a vytiskne vysledek."},
	{"--mexpr",	2, call_mexpr,"Vypocita vyraz A*B*A a vytiskne vysledek."},
	{"--bubbles",	1, call_bubbles,"Spocita bubliny v matici."},
	{"--extbubbles",1, call_bubbles,"Spocita bubliny ve vektoru matic."}
};

/**
 * Chybova hlaseni, ktere funkce mohou vracet
 */
typedef enum {
	E_OK,	// Vse probehlo v poradku
	E_CL,	// Chyba vznikla na prikazove radce
	E_FILE,	// Chyba souboru. Spatny format, neexistujici soubor
	E_MEM,	// Nedostatek pameti
	E_MATH,	// Chyba pri matematicke operaci. Napr. nevhodne rozmery matic
	E_UNKW	// Neznama chyba. Stalo se neco velmi nehezkeho
} error_code;

/**
 * Ke kazdemu chybovemu hlaseni patri slovni popis chyby,
 * ktery se nakonec vypisuje i uzivateli
 */
const char *ERROR_MSG[] = {
	"Vse probehlo bez problemu!",
	"Spatne zadane vstupni parametry!",
	"Nastala chyba pri cteni souboru. Obsahuje validni data?",
	"Nemohu alokovat pamet!",
	"Nastala chyba pri vypoctu. Jsou vstupni data spravna?",
	"Nastala neznama chyba!"
};

/**
 * Hlavicka matice.
 * Obsahuje typ (vektor, matice, vektor matic) a
 * rozmery X, Y, Z.
 */
typedef struct {
	int type;
	int dims[MATRIX_HEADER];
} header_s;

/**
 * Struktura popisuje matici.
 * Hlavicka obsahuje typ a rozmery.
 * Data jsou prvky matice (vektoru, matic}.
 */
typedef struct {
	header_s *header;
	int *data;
} matrix_s;

/**
 * Nacte cislo ze souboru 'fr' a ulozi jej do
 * ukazatele 'number'.
 * V pripade uspechu vraci 'true', jinak 'false'.
 */
bool load_number_from_file(FILE *fr, int *number);

/**
 * Nacte hlavicku matice ze souboru 'fr' do
 * ukazatele 'hdr'. Hlavicka obsahuje typ
 * a rozmery dat.
 *
 * V pripade uspechu vraci true, jinak false.
 */
bool load_header_from_file(FILE *fr, header_s *hdr);

/**
 * Nacte data ze souboru a vrati je v podobe matice.
 * V pripade neuspechu vraci NULL, jinak ukazatel
 * na nove vytvorenou matici.
 */
matrix_s *load_data_from_file(char *filename);

/**
 * Vrati maximalni rozmery matice.
 *
 * Pro vektor vraci hodnotu X,
 * pro matici hodnotu X*Y,
 * pro vektor matic X*Y*Z.
 */
int matrix_get_dimension(header_s *header);

/**
 * Ze zadanych hodnot vytvori hlavicku matice.
 */
header_s matrix_make_header(int typ, int x, int y, int z);

/**
 * Alokuje pamet pro matici o typu a rozmerech, ktere jsou
 * dany v hlavicke 'header'.
 *
 * V pripade chyby vraci NULL, jinak vraci ukazatel na
 * matici o danem typu a rozmerech s vunylovanymi prvky.
 */
matrix_s *matrix_alloc_mem(header_s *header);

/**
 * Uvolni pamet dane matice.
 */
void matrix_free_mem(matrix_s *M);

/**
 * Ulozi cislo 'number' na pozici 'position' v matici 'M'.
 * V pripade uspechu vraci true, jinak false (napriklad pri
 * prekroceni rozmeru matice).
 */
bool matrix_save_number(matrix_s *M, int position, int number);

/**
 * Precte cislo na pozici 'position' z matice 'M' a vrati jej.
 * Pozor! Tato funkce nema jak vratit chybu.
 */
int matrix_get_number(matrix_s *M, int position);

/**
 * Vytiskne matici 'M' na stdout v predem stanovenem
 * formatu.
 * Napriklad matice o rozmerech 2 radky a 3 sloupce:
 *
 * 2
 * 2 3
 * 1 2 3
 * 4 5 6
 */
void matrix_print(matrix_s *M);

/**
 * Vypocita skalarni soucin dvou vektoru. Vysledek vraci v ukazateli 'result'.
 * V pripade uspechu vraci E_OK, jinak cislo chyby, ktera nastala.
 */
int vector_scalar_mul(matrix_s *A, matrix_s *B, int *result);

/**
 * Vypocita soucet dvou vektoru. Vysledek vraci v ukazateli 'result'.
 * V pripade uspechu vraci E_OK, jinak cislo chyby, ktera nastala.
 */
int vector_addition(matrix_s *A, matrix_s *B, matrix_s *C);

/**
 * Vynasobi dve matice 'A' a 'B' a vysledek vrati v 'C'.
 * C je ukazatel na ukazatel matice. Funkce sama si alokuje pamet
 * pro vysledek.
 *
 * V pripade uspechu vraci E_OK, jinak cislo chyby, ktera nastala.
 */
int matrix_multiplication(matrix_s *A, matrix_s *B, matrix_s **C);

/**
 * Urci, jestli v matici 'M' na pozici 'position' a v okoli
 * (2D i 3D varianta) je bublina nebo ne. Kazdy prvek, ktery
 * jiz byl zpracovan je oznacen v matici 'visited', ktera
 * je rozmerove identicka s 'M'.
 *
 * V pripade, ze prvek (a pripadne i okoli) obsahovalo bublinu
 * vraci true, jinak false (false vraci i pokud jsou prekroceny
 * hranice matice, pripadne nastala jina chyba).
 *
 * Tato funkce je volana v matrix_count_bubbles(), pro vypocet
 * bublin v matici pouzivat tu.
 */
bool matrix_bubbles(matrix_s *M, matrix_s *visited, int position);

/**
 * Spocita pocet bublin v matici M a vysledek ulozi do
 * promnenne 'result'. Bublina je definovana jako nula,
 * vsechna ostatni cisla predstavuji material.
 *
 * Pokud vse probehlo bez problemu, vraci E_OK, jinak
 * cislo chyby, ktera nastala.
 */
int matrix_count_bubbles(matrix_s *M, unsigned int *result);

#endif
