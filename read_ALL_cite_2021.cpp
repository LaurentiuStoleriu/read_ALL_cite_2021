#define _CRT_SECURE_NO_WARNINGS 1
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include <errno.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <ctype.h>
#include <string.h>
#include <algorithm>

using namespace std;

#define MANUALE 1					// avem citari puse manual
#undef MANUALE

#define STANDARD_VECHI 1
#undef STANDARD_VECHI
#define an_INI	1900		//interval timp pentru I,C,P
#define an_FIN	3000
#define an_INI5 2016
#define an_FIN5 2020
#define AIS_LIMITA			0.0

int year1 = 1900;
int year2 = 2050;  // year2 >= year1

//Fisierele AIS si IF de pe ISI (AIS cu coloanele inainte de 2006 egalate cu valoarea din 2006)
#define NAI				13714 //14347
#define Nani_AI			23				// 1997 - 2018 !!
#define NAI_LAST_YEAR	2019

//fisierul AIS de pe Eigenfactor
#define NAI_Eig				8077
#define Nani_AI_Eig			18				// 1997 - 2014!!
#define NAI_LAST_YEAR_Eig	2014

#define Nmine			316				// numarul randurilor fisierului din WOS a lucrarilor personale (+header)
#define NciteAUTO		1866			// numarul randurilor fisierului din WOS de citari trase (+header)
#define NciteMAN		0				// numarul randurilor fisierului manual (NO HEADER HERE)						//LS 5
#ifdef MANUALE
#define Ncite			(NciteAUTO+NciteMAN)
#else
#define Ncite			NciteAUTO
#endif
#define Ncitecite		1000000			// numarul total de referinte individuale ale tuturor celor Ncite articole trase

#define Cols				67//66//62				// numarul de coloane dat de WOS

#define COL_Authors			1	// $AU$
#define COL_Title			8	// $TI$
#define COL_ReprintAuthor	23	// $RP$
#define COL_Citari			29	// $CR$
#define COL_TimesCited		31	// $TC$
#define COL_Journal			41	// $J9$
#define COL_Year			44	// $PY$
#define COL_Volume			45	// $VL$
#define COL_Issue			46	// $IS$
#define COL_PagStart		51	// $BP$
#define COL_PagEnd			52	// $EP$
#define COL_ArtNo			53	// $AR$
#define COL_DOI				54	// $DI$

string prim_autor = "stancu";
char path_IN_own[] = "E:\\Stoleriu\\Doc\\CV\\auto\\AS_savedrecs_mine.txt";
char path_IN_cites[] = "E:\\Stoleriu\\Doc\\CV\\auto\\AS_savedrecs.txt";
char path_IN_cites_manual[] = "E:\\Stoleriu\\Doc\\CV\\auto\\LS_savedrecs_citari-manual.txt";

char path_IN_AI[] = "E:\\Stoleriu\\Doc\\CV\\auto\\AI_factor_full.txt"; // are coloana pentru 2019 duplicata dupa 2018 - 23 de coloane
char path_IN_IF[] = "E:\\Stoleriu\\Doc\\CV\\auto\\IF_factor_full.txt";
char path_IN_AIeig[] = "E:\\Stoleriu\\Doc\\CV\\auto\\AIS_eigen_97-06.txt"; // fisierul de pe Eigenfactor pentru ani mai mici de 2006

char path_OUT_total[] = "E:\\Stoleriu\\Doc\\CV\\auto\\AS_total.txt";
char path_OUT_total5[] = "E:\\Stoleriu\\Doc\\CV\\auto\\AS_total5.txt";
char path_OUT_UAIC[] = "E:\\Stoleriu\\Doc\\CV\\auto\\AS_UAIC5.txt";
char path_OUT_IP[] = "E:\\Stoleriu\\Doc\\CV\\auto\\AS_IP.txt";
char path_OUT_C[] = "E:\\Stoleriu\\Doc\\CV\\auto\\AS_C.txt";
//char path_OUT_IP5[] = "E:\\Stoleriu\\Doc\\CV\\auto\\OC_IP5.txt";
//char path_OUT_C5[] = "E:\\Stoleriu\\Doc\\CV\\auto\\OC_C5.txt";

char buffer[110001];
string mine[Cols], cite[Cols];
char DOImine[Nmine][100], TCsscanf[100];
int TCmine[Nmine], CiteFoundmine[Nmine];
int contorDOIs[Ncite];
char DOIs[Ncite][1000][100]; // era [1000][100]

double P, I, C, UAIC_0, UAIC_1, UAIC_2, P5, I5, C5;

struct ISI
{
	std::string journal;
	double IF, AIS;
};

struct ref_struct
{
	std::string name, first_author, s_year, journal, s_vol, s_pag, DOI;
	int index_paper, index_ref, year;
};
struct own
{
	std::string name, first_author, title, s_year, journal, s_vol, s_issue, s_pag_start, s_pag_end, s_art_no, DOI;
	int year, TimesCited, NA, prim;
	double NAeff, IF, AIS;
};

string refs[Ncite];

struct own own_papers[Nmine];
struct own others_papers[Ncite];

struct ref_struct refs_individ[Ncitecite];
std::string AI_journals_name[NAI];
std::string IF_journals_name[NAI];
double AI_journals_AI[NAI][Nani_AI];
double IF_journals_IF[NAI][Nani_AI];

std::string AI_journals_Eig_name[NAI_Eig];
double AI_journals_Eig[NAI_Eig][Nani_AI_Eig];

int main(void)
{
	int i, j, k;
	FILE *fp_in_mine, *fp_in_cite, *fp_in_cite_MAN, *fp_in_ISI;

	P = 0.0;
	I = 0.0;
	C = 0.0;
	UAIC_0 = 0.0;
	UAIC_1 = 0.0;
	UAIC_2 = 0.0;

	char tempchar[500];

	fopen_s(&fp_in_ISI, path_IN_AI, "r");
	for (i = 0; i < NAI; i++)
	{
		fscanf(fp_in_ISI, "%[^\t]s", &tempchar[0]);
		AI_journals_name[i] = tempchar;
		if (!isalpha(AI_journals_name[i][0]))
			AI_journals_name[i].erase(0, 1);
		for (j = 0; j < Nani_AI; j++)
		{
			fscanf(fp_in_ISI, "%lf", &AI_journals_AI[i][j]);
		}
	}
	fclose(fp_in_ISI);

	fopen_s(&fp_in_ISI, path_IN_IF, "r");
	for (i = 0; i < NAI; i++)
	{
		fscanf(fp_in_ISI, "%[^\t]s", &tempchar[0]);
		IF_journals_name[i] = tempchar;
		if (!isalpha(IF_journals_name[i][0]))
			IF_journals_name[i].erase(0, 1);
		for (j = 0; j < Nani_AI/*-1*/; j++)
		{
			fscanf(fp_in_ISI, "%lf", &IF_journals_IF[i][j]);
		}
		//IF_journals_IF[i][Nani_AI - 1] = IF_journals_IF[i][Nani_AI - 2];
	}
	fclose(fp_in_ISI);

	fopen_s(&fp_in_ISI, path_IN_AIeig, "r");
	for (i = 0; i < NAI_Eig; i++)
	{
		fscanf(fp_in_ISI, "%[^\t]s", &tempchar[0]);
		AI_journals_Eig_name[i] = tempchar;
		if (!isalpha(AI_journals_Eig_name[i][0]))
			AI_journals_Eig_name[i].erase(0, 1);
		for (j = 0; j < Nani_AI_Eig; j++)
		{
			fscanf(fp_in_ISI, "%lf", &AI_journals_Eig[i][j]);
		}
	}
	fclose(fp_in_ISI);

	////////////////////////////////////////////////////////////////////////// organizare OWN

	fopen_s(&fp_in_mine, path_IN_own, "r");

	int searchpos = 0;
	int contor_lucrari_eliminate_limita_ani = 0;
	int contor_citari_eliminate_limita_ani = 0;
	for (i = 0; i < (Nmine - contor_lucrari_eliminate_limita_ani); i++)
	{
		for (j = 0; j < Cols; j++)
		{
			fscanf(fp_in_mine, "%[^\t] ", &buffer);
			mine[j] = buffer;
		}

		if (i == 0)	//sare primul rand (antet)
		{
			continue;
		}
		sscanf(mine[COL_DOI].c_str(), "%s ", &DOImine[i][0]);
		sscanf(mine[COL_TimesCited].c_str(), "$%[^$] ", &TCsscanf);
		TCmine[i] = atoi(TCsscanf);

		own_papers[i - 1].name = mine[1];
		own_papers[i - 1].name.erase(0, 1);
		own_papers[i - 1].name.erase(own_papers[i - 1].name.size() - 1, 1);
		own_papers[i - 1].NA = 1;
#ifndef STANDARD_VECHI
		while ((searchpos = own_papers[i - 1].name.find(';', searchpos + 1)) > 0)
		{
			own_papers[i - 1].NA++;
		}

		if (own_papers[i - 1].NA <= 5)
		{
			own_papers[i - 1].NAeff = (double)own_papers[i - 1].NA;
		}
		else
		{
			if (own_papers[i - 1].NA <= 15)
			{
				own_papers[i - 1].NAeff = ((double)own_papers[i - 1].NA + 5.0) / 2.0;
			}
			else
			{
				if (own_papers[i - 1].NA <= 75)
				{
					own_papers[i - 1].NAeff = ((double)own_papers[i - 1].NA + 15.0) / 3.0;
				}
				else
				{
					own_papers[i - 1].NAeff = ((double)own_papers[i - 1].NA + 45.0) / 4.0;
				}
			}
		}
#else
		while ((searchpos = own_papers[i - 1].name.find(';', searchpos + 1)) > 0)
		{
			own_papers[i - 1].NA++;
		}
		if (own_papers[i - 1].NA < 5)
		{
			own_papers[i - 1].NAeff = (double)own_papers[i - 1].NA;
		}
		else
		{
			own_papers[i - 1].NAeff = ((double)own_papers[i - 1].NA + 10.0) / 3.0;
		}
#endif

		own_papers[i - 1].first_author = own_papers[i - 1].name.substr(0, own_papers[i - 1].name.find(','));

		string corresp = mine[COL_ReprintAuthor]/*mine[i][COL_ReprintAuthor]*/;
		transform(corresp.begin(), corresp.end(), corresp.begin(), tolower);
		string prim_autor_in_articol = own_papers[i - 1].first_author;
		transform(prim_autor_in_articol.begin(), prim_autor_in_articol.end(), prim_autor_in_articol.begin(), tolower);
		int temp_pos_corresp = 0;

		if ((prim_autor_in_articol == prim_autor))
			//if ( (own_papers[i-1].first_author == "Stancu") || (own_papers[i-1].first_author == "STANCU") )
				//if (own_papers[i-1].first_author == "Enachescu")
			own_papers[i - 1].prim = 1;
		else
		{
			temp_pos_corresp = corresp.find(prim_autor);
			if (temp_pos_corresp >= 0)
			{
				own_papers[i - 1].prim = 1;
			}
			else
			{
				own_papers[i - 1].prim = 0;
			}
		}

		own_papers[i - 1].title = mine[COL_Title];
		own_papers[i - 1].title.erase(0, 1);
		own_papers[i - 1].title.erase(own_papers[i - 1].title.size() - 1, 1);

		own_papers[i - 1].s_year = mine[COL_Year];
		own_papers[i - 1].s_year.erase(0, 1);
		own_papers[i - 1].s_year.erase(own_papers[i - 1].s_year.size() - 1, 1);
		own_papers[i - 1].year = atoi(own_papers[i - 1].s_year.c_str());

		if ((own_papers[i - 1].year < an_INI) || (own_papers[i - 1].year > an_FIN))
		{
			i--;
			contor_lucrari_eliminate_limita_ani++;
			continue;
		}

		own_papers[i - 1].journal = mine[COL_Journal];
		own_papers[i - 1].journal.erase(0, 1);
		own_papers[i - 1].journal.erase(own_papers[i - 1].journal.size() - 1, 1);

		own_papers[i - 1].s_vol = mine[COL_Volume];
		own_papers[i - 1].s_vol.erase(0, 1);
		own_papers[i - 1].s_vol.erase(own_papers[i - 1].s_vol.size() - 1, 1);

		own_papers[i - 1].s_issue = mine[COL_Issue];
		own_papers[i - 1].s_issue.erase(0, 1);
		own_papers[i - 1].s_issue.erase(own_papers[i - 1].s_issue.size() - 1, 1);

		own_papers[i - 1].s_pag_start = mine[COL_PagStart];
		own_papers[i - 1].s_pag_start.erase(0, 1);
		own_papers[i - 1].s_pag_start.erase(own_papers[i - 1].s_pag_start.size() - 1, 1);

		own_papers[i - 1].s_pag_end = mine[COL_PagEnd];
		own_papers[i - 1].s_pag_end.erase(0, 1);
		own_papers[i - 1].s_pag_end.erase(own_papers[i - 1].s_pag_end.size() - 1, 1);

		own_papers[i - 1].s_art_no = mine[COL_ArtNo];
		own_papers[i - 1].s_art_no.erase(0, 1);
		own_papers[i - 1].s_art_no.erase(own_papers[i - 1].s_art_no.size() - 1, 1);

		own_papers[i - 1].DOI = mine[COL_DOI];
		own_papers[i - 1].DOI.erase(0, 1);
		own_papers[i - 1].DOI.erase(own_papers[i - 1].DOI.size() - 1, 1);

		transform(own_papers[i - 1].DOI.begin(), own_papers[i - 1].DOI.end(), own_papers[i - 1].DOI.begin(), ::toupper);

		own_papers[i - 1].TimesCited = TCmine[i];

		own_papers[i - 1].IF = 0.0;
		own_papers[i - 1].AIS = 0.0;

		for (j = 0; j < NAI; j++)
		{
			if (own_papers[i - 1].journal == IF_journals_name[j])
			{
				if (own_papers[i - 1].year <= NAI_LAST_YEAR)
				{
					own_papers[i - 1].IF = IF_journals_IF[j][own_papers[i - 1].year - 1997];
				}
				else
				{
					own_papers[i - 1].IF = IF_journals_IF[j][Nani_AI - 1];
				}
				break;
			}
		}

		// 		if (own_papers[i - 1].year < 2007)	// din Eigenfactor
		// 		{
		// 			for (j = 0; j < NAI_Eig; j++)
		// 			{
		// 				if (own_papers[i - 1].journal == AI_journals_Eig_name[j])
		// 				{
		// 					own_papers[i - 1].AIS = AI_journals_Eig[j][own_papers[i - 1].year - 1997];
		// 					break;
		// 				}
		// 			}
		// 		}
		// 		else								// din ISI
		// 		{
		for (j = 0; j < NAI; j++)
		{
			if (own_papers[i - 1].journal == AI_journals_name[j])
			{
				if (own_papers[i - 1].year < NAI_LAST_YEAR)
				{
					own_papers[i - 1].AIS = AI_journals_AI[j][own_papers[i - 1].year - 1997];
				}
				else
				{
					own_papers[i - 1].AIS = AI_journals_AI[j][Nani_AI - 1];
				}
				break;
			}
		}
		//}

		printf("lucrarea mea nr. %d - %d TC \n %s, %s, %s, %s",
			i, own_papers[i - 1].TimesCited,
			own_papers[i - 1].name.c_str(), own_papers[i - 1].title.c_str(), own_papers[i - 1].journal.c_str(), own_papers[i - 1].s_vol.c_str());
		if (strcmp(own_papers[i - 1].s_issue.c_str(), "")) //daca exista
		{
			printf("(%s), ", own_papers[i - 1].s_issue.c_str());
		}
		else
		{
			printf(", ");
		}
		if (strcmp(own_papers[i - 1].s_pag_start.c_str(), "")) //daca exista
		{
			printf("pp. %s", own_papers[i - 1].s_pag_start.c_str());
			if (strcmp(own_papers[i - 1].s_pag_end.c_str(), "")) //daca exista
			{
				printf("-%s, ", own_papers[i - 1].s_pag_end.c_str());
			}
			else
			{
				printf(", ");
			}
		}
		if (strcmp(own_papers[i - 1].s_art_no.c_str(), "")) //daca exista
		{
			printf("art.no. %s, ", own_papers[i - 1].s_art_no.c_str());
		}

		printf("(%d) - DOI: %s\n", own_papers[i - 1].year, own_papers[i - 1].DOI.c_str());
	}

	fclose(fp_in_mine);
	printf("\n%d lucrari sarite din cauza limitei de ani (%d, %d)\n", contor_lucrari_eliminate_limita_ani, an_INI, an_FIN);
	printf("************************\n");

	////////////////////////////////////////////////////////////////////////// organizare CITES

	fopen_s(&fp_in_cite, path_IN_cites, "r");
#ifdef MANUALE
	fopen_s(&fp_in_cite_MAN, path_IN_cites_manual, "r");
#endif // MANUALE

	for (i = 0; i < Ncite; i++)
	{
#ifdef MANUALE
		if (i < NciteAUTO)
		{
			for (j = 0; j < Cols; j++)
			{
				fscanf(fp_in_cite, "%110000[^\t] ", &buffer);
				cite[j] = buffer;
			}
			if (i == 0)
			{
				continue;
			}
		}
		else
		{
			for (j = 0; j < Cols; j++)
			{
				fscanf(fp_in_cite_MAN, "%110000[^\t] ", &buffer);
				cite[j] = buffer;
			}
		}
#else
		for (j = 0; j < Cols; j++)
		{
			fscanf(fp_in_cite, "%110000[^\t] ", &buffer);
			cite[j] = buffer;
		}
		if (i == 0)
		{
			continue;
		}
#endif

		j = 0;
		contorDOIs[i] = 0;

		int pozitie_DOI_veche = 0;
		int pozitie_DOI = 0;
		int pozitie_end_DOI = 0;
		string token_begin = "DOI";
		string token_end = ";";
		string last_token = "$";
		string substring_DOI;

		while (pozitie_DOI != string::npos)
		{
			pozitie_DOI = cite[COL_Citari].find(token_begin, pozitie_DOI_veche);
			if (pozitie_DOI != string::npos)
			{
				pozitie_DOI_veche = pozitie_DOI + 1;
				pozitie_end_DOI = cite[COL_Citari].find(token_end, pozitie_DOI_veche);
				if (pozitie_end_DOI == string::npos)
				{
					pozitie_end_DOI = cite[COL_Citari].find(last_token, pozitie_DOI_veche);
				}

				substring_DOI = cite[COL_Citari].substr(pozitie_DOI, pozitie_end_DOI - pozitie_DOI);

				contorDOIs[i]++;
				sprintf(DOIs[i][contorDOIs[i] - 1], "$%s$", substring_DOI.c_str());
			}
		}

		//while (cite[COL_Citari][j]!='\0')
		//{
		//	if ( (cite[COL_Citari][j]=='D') && (cite[COL_Citari][j+1]=='O') && (cite[COL_Citari][j+2]=='I') )
		//	{
		//		contorDOIs[i]++;
		//		k=0;
		//		DOIs[i][contorDOIs[i]-1][k]='$';
		//		while (cite[COL_Citari][j+4+k]!=';')
		//		{
		//			k++;
		//			DOIs[i][contorDOIs[i]-1][k] = cite[COL_Citari][j+3+k];
		//		}
		//		DOIs[i][contorDOIs[i]-1][k+1] = '$';
		//		DOIs[i][contorDOIs[i]-1][k+2] = '\0';
		//		j += k;
		//	}
		//	else
		//		j++;
		//}
	}
	_fcloseall();

	for (i = 1; i < Ncite; i++)
	{
		printf("cite %d - %d DOIs: \n", i, contorDOIs[i]);
		//for (j=0;j<contorDOIs[i];j++)
		//{
		//	printf("\t\t%s\n", DOIs[i][j]);
		//}
	}

	cout << "Check DOIs and PRESS A KEY" << endl;
	getchar();

	fopen_s(&fp_in_cite, path_IN_cites, "r");
#ifdef MANUALE
	fopen_s(&fp_in_cite_MAN, path_IN_cites_manual, "r");
#endif // MANUALE
	for (i = 0; i < Ncite; i++)
	{
#ifdef MANUALE
		if (i < NciteAUTO)
		{
			for (j = 0; j < Cols; j++)
			{
				fscanf(fp_in_cite, "%60000[^\t] ", &buffer);
				cite[j] = buffer;
			}
			if (i == 0)
			{
				continue;
			}
		}
		else
		{
			for (j = 0; j < Cols; j++)
			{
				fscanf(fp_in_cite_MAN, "%60000[^\t] ", &buffer);
				cite[j] = buffer;
			}
		}
#else
		for (j = 0; j < Cols; j++)
		{
			fscanf(fp_in_cite, "%110000[^\t] ", &buffer);
			cite[j] = buffer;
		}
		if (i == 0)
		{
			continue;
		}
#endif

		others_papers[i - 1].name = cite[COL_Authors];
		others_papers[i - 1].name.erase(0, 1);
		others_papers[i - 1].name.erase(others_papers[i - 1].name.size() - 1, 1);

		others_papers[i - 1].first_author = others_papers[i - 1].name.substr(0, others_papers[i - 1].name.find(','));

		others_papers[i - 1].title = cite[COL_Title];
		others_papers[i - 1].title.erase(0, 1);
		others_papers[i - 1].title.erase(others_papers[i - 1].title.size() - 1, 1);

		others_papers[i - 1].s_year = cite[COL_Year];
		others_papers[i - 1].s_year.erase(0, 1);
		others_papers[i - 1].s_year.erase(others_papers[i - 1].s_year.size() - 1, 1);
		others_papers[i - 1].year = atoi(others_papers[i - 1].s_year.c_str());

		others_papers[i - 1].journal = cite[COL_Journal];
		others_papers[i - 1].journal.erase(0, 1);
		others_papers[i - 1].journal.erase(others_papers[i - 1].journal.size() - 1, 1);

		others_papers[i - 1].s_vol = cite[COL_Volume];
		others_papers[i - 1].s_vol.erase(0, 1);
		others_papers[i - 1].s_vol.erase(others_papers[i - 1].s_vol.size() - 1, 1);

		others_papers[i - 1].s_issue = cite[COL_Issue];
		others_papers[i - 1].s_issue.erase(0, 1);
		others_papers[i - 1].s_issue.erase(others_papers[i - 1].s_issue.size() - 1, 1);

		others_papers[i - 1].s_pag_start = cite[COL_PagStart];
		others_papers[i - 1].s_pag_start.erase(0, 1);
		others_papers[i - 1].s_pag_start.erase(others_papers[i - 1].s_pag_start.size() - 1, 1);

		others_papers[i - 1].s_pag_end = cite[COL_PagEnd];
		others_papers[i - 1].s_pag_end.erase(0, 1);
		others_papers[i - 1].s_pag_end.erase(others_papers[i - 1].s_pag_end.size() - 1, 1);

		others_papers[i - 1].s_art_no = cite[COL_ArtNo];
		others_papers[i - 1].s_art_no.erase(0, 1);
		others_papers[i - 1].s_art_no.erase(others_papers[i - 1].s_art_no.size() - 1, 1);

		others_papers[i - 1].DOI = cite[COL_DOI];
		others_papers[i - 1].DOI.erase(0, 1);
		others_papers[i - 1].DOI.erase(others_papers[i - 1].DOI.size() - 1, 1);

		transform(others_papers[i - 1].DOI.begin(), others_papers[i - 1].DOI.end(), others_papers[i - 1].DOI.begin(), ::toupper);

		others_papers[i - 1].IF = 0.0;
		others_papers[i - 1].AIS = 0.0;

		for (j = 0; j < NAI; j++)
		{
			if (others_papers[i - 1].journal == IF_journals_name[j])
			{
				if (others_papers[i - 1].year <= NAI_LAST_YEAR)
				{
					others_papers[i - 1].IF = IF_journals_IF[j][others_papers[i - 1].year - 1997];
				}
				else
				{
					others_papers[i - 1].IF = IF_journals_IF[j][Nani_AI - 1];
				}
				break;
			}
		}

		// 		for (j = 0; j < NAI; j++)
		// 		{
		// 			if (others_papers[i - 1].journal == AI_journals_name[j])
		// 			{
		// 				if (others_papers[i - 1].year <= NAI_LAST_YEAR)
		// 				{
		// 					others_papers[i - 1].AIS = AI_journals_AI[j][others_papers[i - 1].year - 1997];
		// 				}
		// 				else
		// 				{
		// 					others_papers[i - 1].AIS = AI_journals_AI[j][Nani_AI - 1];
		// 				}
		// 				break;
		// 			}
		// 		}
		if (others_papers[i - 1].year < 2007)	// din Eigenfactor
		{
			for (j = 0; j < NAI_Eig; j++)
			{
				if (others_papers[i - 1].journal == AI_journals_Eig_name[j])
				{
					others_papers[i - 1].AIS = AI_journals_Eig[j][others_papers[i - 1].year - 1997];
					break;
				}
			}
		}
		else								// din ISI
		{
			for (j = 0; j < NAI; j++)
			{
				if (others_papers[i - 1].journal == AI_journals_name[j])
				{
					if (others_papers[i - 1].year < NAI_LAST_YEAR)
					{
						others_papers[i - 1].AIS = AI_journals_AI[j][others_papers[i - 1].year - 1997];
					}
					else
					{
						others_papers[i - 1].AIS = AI_journals_AI[j][Nani_AI - 1];
					}
					break;
				}
			}
		}

		printf("lucrarea care citeaza nr. %d  \n %s, %s, %s, %s",
			i, others_papers[i - 1].name.c_str(), others_papers[i - 1].title.c_str(), others_papers[i - 1].journal.c_str(), others_papers[i - 1].s_vol.c_str());
		if (strcmp(others_papers[i - 1].s_issue.c_str(), "")) //daca exista
		{
			printf("(%s), ", others_papers[i - 1].s_issue.c_str());
		}
		else
		{
			printf(", ");
		}
		if (strcmp(others_papers[i - 1].s_pag_start.c_str(), "")) //daca exista
		{
			printf("pp. %s", others_papers[i - 1].s_pag_start.c_str());
			if (strcmp(others_papers[i - 1].s_pag_end.c_str(), "")) //daca exista
			{
				printf("-%s, ", others_papers[i - 1].s_pag_end.c_str());
			}
			else
			{
				printf(", ");
			}
		}
		if (strcmp(others_papers[i - 1].s_art_no.c_str(), "")) //daca exista
		{
			printf("art.no. %s, ", others_papers[i - 1].s_art_no.c_str());
		}

		printf("(%d) - DOI: %s\n", others_papers[i - 1].year, others_papers[i - 1].DOI.c_str());
	}
	_fcloseall();
	printf("************************\n");

	////////////////////////////////////////////////////////////////////////// separare CitesCites (field [COL_Citari])
	int contor_cites_general = 1;
	int contor_cites_local = 0;
	int	contor_fields_in_record = 0;
	int lastpos = 0;
	int newpos = 0;
	int lastpos_general = 0;
	int newpos_general = 0;

	FILE *fp_out_cite, *fp_out_total, *fp_out_cite5, *fp_out_total5;

	contor_cites_general = 1;

	string ref_individ_temp;
	string record_temp;

	fopen_s(&fp_in_cite, path_IN_cites, "r");
#ifdef MANUALE
	fopen_s(&fp_in_cite_MAN, path_IN_cites_manual, "r");
#endif // MANUALE
	for (i = 0; i < Ncite; i++)
	{
#ifdef MANUALE
		if (i < NciteAUTO)
		{
			for (j = 0; j < Cols; j++)
			{
				fscanf(fp_in_cite, "%60000[^\t] ", &buffer);
				cite[j] = buffer;
			}
			if (i == 0)
			{
				continue;
			}
		}
		else
		{
			for (j = 0; j < Cols; j++)
			{
				fscanf(fp_in_cite_MAN, "%60000[^\t] ", &buffer);
				cite[j] = buffer;
			}
		}
#else
		for (j = 0; j < Cols; j++)
		{
			fscanf(fp_in_cite, "%110000[^\t] ", &buffer);
			cite[j] = buffer;
		}
		if (i == 0)
		{
			continue;
		}
#endif
		int GATA_general = 0;
		int GATA = 0;
		contor_cites_local = 0;
		refs[i - 1] = cite[COL_Citari];
		lastpos_general = 0;
		newpos_general = 0;
		newpos_general = refs[i - 1].find(";", lastpos_general);

		//while( newpos_general != -1 )
		while (!GATA_general)
		{
			if (newpos_general == -1)
			{
				newpos_general = refs[i - 1].length();//ref_individ_temp.find("$", lastpos);
				GATA_general = 1;
			}

			contor_cites_local++;
			if (!GATA_general)
				ref_individ_temp = refs[i - 1].substr(lastpos_general, newpos_general - lastpos_general);
			else
				ref_individ_temp = refs[i - 1].substr(lastpos_general, newpos_general - lastpos_general - 1);

			///////////////CLEAR UTF
			for (int loco = 0; loco < ref_individ_temp.length(); loco++)
			{
				if (ref_individ_temp[loco] < 0)
					ref_individ_temp[loco] = 32;				/////spatiu
			}
			////////////////

			//printf( "working on %d, (%d a lucrarii %d)\n", contor_cites_general, contor_cites_local, i );
			refs_individ[contor_cites_general - 1].index_paper = i;
			refs_individ[contor_cites_general - 1].index_ref = contor_cites_local;

			lastpos = 0;
			contor_fields_in_record = 0;

			newpos = ref_individ_temp.find(",", lastpos);
			//while ( (newpos != -1)  )
			while (!GATA)
			{
				if (newpos == -1)
				{
					newpos = ref_individ_temp.length();//ref_individ_temp.find("$", lastpos);
					GATA = 1;
				}
				contor_fields_in_record++;
				record_temp = ref_individ_temp.substr(lastpos + 1, newpos - (lastpos + 1));

				record_temp.erase(remove_if(record_temp.begin(), record_temp.end(), isspace), record_temp.end());	// remove leading spaces

				switch (contor_fields_in_record)
				{
				case 1:
					refs_individ[contor_cites_general - 1].name = record_temp;
					refs_individ[contor_cites_general - 1].first_author = refs_individ[contor_cites_general - 1].name.substr(0, refs_individ[contor_cites_general - 1].name.find(' '));
					break;
				case 2:
					if ((record_temp[0] == '1') || (record_temp[0] == '2'))
					{
						refs_individ[contor_cites_general - 1].s_year = record_temp;
						refs_individ[contor_cites_general - 1].year = atoi(refs_individ[contor_cites_general - 1].s_year.c_str());
					}
					else
					{
						refs_individ[contor_cites_general - 1].journal = record_temp;
					}
					break;
				case 3:
					switch (record_temp[0])
					{
					case 'V':
						refs_individ[contor_cites_general - 1].s_vol = record_temp;
						refs_individ[contor_cites_general - 1].s_vol.erase(refs_individ[contor_cites_general - 1].s_vol.begin());
						break;
					case 'D':
						if (record_temp[1] == 'O')
						{
							refs_individ[contor_cites_general - 1].DOI = record_temp;
							refs_individ[contor_cites_general - 1].DOI.erase(refs_individ[contor_cites_general - 1].DOI.begin(), refs_individ[contor_cites_general - 1].DOI.begin() + 4);
						}
						break;
					default:
						refs_individ[contor_cites_general - 1].journal = record_temp;
						break;
					}
					break;
				case 4:
				case 5:
				case 6:
				case 7:
					switch (record_temp[0])
					{
					case 'V':
						refs_individ[contor_cites_general - 1].s_vol = record_temp;
						refs_individ[contor_cites_general - 1].s_vol.erase(0, 1/*refs_individ[contor_cites_general-1].s_vol.begin()*/);
						break;
					case 'P':
					case 'p':
						refs_individ[contor_cites_general - 1].s_pag = record_temp;
						refs_individ[contor_cites_general - 1].s_pag.erase(0, 1/*refs_individ[contor_cites_general-1].s_pag.begin()*/);
						break;
					case 'D':
						refs_individ[contor_cites_general - 1].DOI = record_temp;
						refs_individ[contor_cites_general - 1].DOI.erase(0, 3/*refs_individ[contor_cites_general-1].DOI.begin(), refs_individ[contor_cites_general-1].DOI.begin()+4*/);
						if (refs_individ[contor_cites_general - 1].DOI[0] == '[')
						{
							refs_individ[contor_cites_general - 1].DOI.erase(0, 1/*refs_individ[contor_cites_general-1].DOI.begin(), refs_individ[contor_cites_general-1].DOI.begin()+1*/);
						}
						break;
					default:

						printf("working on %d, (%d a lucrarii %d)\n", contor_cites_general, contor_cites_local, i);
						printf("MIGHT NOT WORK! Double DOI? %s   %s   %s\n", refs_individ[contor_cites_general - 1].first_author.c_str(), refs_individ[contor_cites_general - 1].s_year.c_str(), refs_individ[contor_cites_general - 1].DOI.c_str());
						break;
					}
					break;
				}
				lastpos = newpos + 1;
				newpos = ref_individ_temp.find(",", lastpos);
			}
			GATA = 0;

			//if (contor_cites_general > 9800)
			//	printf("%d\n", contor_cites_general);
			contor_cites_general++;
			lastpos_general = newpos_general + 1;
			newpos_general = refs[i - 1].find(";", lastpos_general);
		}
		GATA_general = 0;
	}

	_fcloseall();

	int TOTAL_CITES_MINE = 0;
	int SCRIE = 0;
	int OBS = 0;

	//fopen_s(&fp_out_cite,"E:\\Stoleriu\\Doc\\CV\\auto\\LS_cites.txt", "w");
	//fprintf(fp_out_cite, "ID2 \t ID1 \t NAME \t TITLE \t JOURNAL \t VOLUME \t ISSUE \t PAGE_START \t PAGE_END \t ART_NO \t YEAR \t DOI \t OBS\n");
	for (i = 1; i < (Nmine - contor_lucrari_eliminate_limita_ani); i++)
	{
		CiteFoundmine[i] = 0;
		SCRIE = 0;
		OBS = 0;
		for (j = 0; j < contor_cites_general - 1; j++)
		{
			transform(refs_individ[j].DOI.begin(), refs_individ[j].DOI.end(), refs_individ[j].DOI.begin(), toupper);
			//refs_individ[j].DOI.erase(remove_if(refs_individ[j].DOI.begin(), refs_individ[j].DOI.end(), isspace), refs_individ[j].DOI.end());
			//remove_if(refs_individ[j].DOI.begin(), refs_individ[j].DOI.end(), isspace);

			if ((refs_individ[j].DOI != "") && (refs_individ[j].DOI == own_papers[i].DOI)) // gasite cu DOI
			{
				CiteFoundmine[i]++;
				TOTAL_CITES_MINE++;
				SCRIE = 1;
				OBS = 0;
			}
			else
			{
				if ((refs_individ[j].DOI == ""))											// nu are DOI
				{
					transform(refs_individ[j].first_author.begin(), refs_individ[j].first_author.end(), refs_individ[j].first_author.begin(), tolower);
					transform(own_papers[i].first_author.begin(), own_papers[i].first_author.end(), own_papers[i].first_author.begin(), tolower);
					if (((refs_individ[j].first_author.c_str()) == own_papers[i].first_author.c_str()) &&
						(refs_individ[j].journal.c_str() == own_papers[i].journal.c_str()) &&
						(refs_individ[j].year == own_papers[i].year))						// aceleasi
					{
						if ((refs_individ[j].s_pag != "") && (own_papers[i].s_pag_start != "") && (refs_individ[j].s_pag == own_papers[i].s_pag_start))
						{
							CiteFoundmine[i]++;
							TOTAL_CITES_MINE++;
							SCRIE = 1;
							OBS = 1;
						}
						else
						{
							if ((refs_individ[j].s_pag != "") && (own_papers[i].s_art_no != "") && (refs_individ[j].s_pag == own_papers[i].s_art_no))
							{
								CiteFoundmine[i]++;
								TOTAL_CITES_MINE++;
								SCRIE = 1;
								OBS = 1;
							}
						}
					}
				}
			}
		}
	}
	//fclose(fp_out_cite);
	printf("************************\n");
	printf("TOTAL_CITES_MINE found %d\n", TOTAL_CITES_MINE);
	//////////////////////////////////////////////////////////////////////////  SAVE all
	double prim, inf, cit;
	double prim5, inf5, cit5;
	int no_citari_valide = 0;
	int no_citari_valide5 = 0;

	//FILE *fp_reviste_mine, *fp_reviste_cite;

	//fopen_s(&fp_reviste_mine, path_OUT_REVISTE_MINE, "w");
	//fopen_s(&fp_reviste_cite, path_OUT_REVISTE_CITE, "w");
	fopen_s(&fp_out_total, path_OUT_total, "w");
	fopen_s(&fp_out_total5, path_OUT_total5, "w");
	//fprintf(fp_out_total, "ID1 \t NAME \t TITLE \t JOURNAL \t VOLUME \t ISSUE \t PAGE_START \t PAGE_END \t ART_NO \t YEAR \t DOI\n");
	for (i = 1; i < (Nmine - contor_lucrari_eliminate_limita_ani); i++)
	{
		prim = 0;
		inf = 0;
		cit = 0;
		prim5 = 0;
		inf5 = 0;
		cit5 = 0;
		//fprintf(fp_reviste_mine, "%s\n", own_papers[i - 1].journal.c_str());

		fprintf(fp_out_total, "[%d] \t %s, %s, %s, vol. %s", i, own_papers[i - 1].name.c_str(), own_papers[i - 1].title.c_str(), own_papers[i - 1].journal.c_str(), own_papers[i - 1].s_vol.c_str());
		fprintf(fp_out_total5, "[%d] \t %s, %s, %s, vol. %s", i, own_papers[i - 1].name.c_str(), own_papers[i - 1].title.c_str(), own_papers[i - 1].journal.c_str(), own_papers[i - 1].s_vol.c_str());
		if (strcmp(own_papers[i - 1].s_issue.c_str(), "")) //daca exista
		{
			fprintf(fp_out_total, "(%s), ", own_papers[i - 1].s_issue.c_str());
			fprintf(fp_out_total5, "(%s), ", own_papers[i - 1].s_issue.c_str());
		}
		else
		{
			fprintf(fp_out_total, ", ");
			fprintf(fp_out_total5, ", ");
		}
		if (strcmp(own_papers[i - 1].s_pag_start.c_str(), "")) //daca exista
		{
			fprintf(fp_out_total, "pp. %s", own_papers[i - 1].s_pag_start.c_str());
			fprintf(fp_out_total5, "pp. %s", own_papers[i - 1].s_pag_start.c_str());
			if (strcmp(own_papers[i - 1].s_pag_end.c_str(), "")) //daca exista
			{
				fprintf(fp_out_total, "-%s, ", own_papers[i - 1].s_pag_end.c_str());
				fprintf(fp_out_total5, "-%s, ", own_papers[i - 1].s_pag_end.c_str());
			}
			else
			{
				fprintf(fp_out_total, ", ");
				fprintf(fp_out_total5, ", ");
			}
		}
		if (strcmp(own_papers[i - 1].s_art_no.c_str(), "")) //daca exista
		{
			fprintf(fp_out_total, "art.no. %s, ", own_papers[i - 1].s_art_no.c_str());
			fprintf(fp_out_total5, "art.no. %s, ", own_papers[i - 1].s_art_no.c_str());
		}

		fprintf(fp_out_total, "(%d) %s\n", own_papers[i - 1].year, own_papers[i - 1].DOI.c_str());
		fprintf(fp_out_total5, "(%d) %s\n", own_papers[i - 1].year, own_papers[i - 1].DOI.c_str());

		if (own_papers[i - 1].AIS >= AIS_LIMITA)
		{
			if (own_papers[i - 1].prim)
			{
				prim = own_papers[i - 1].AIS;
				if ((own_papers[i - 1].year <= an_FIN5) && (own_papers[i - 1].year >= an_INI5))
				{
					prim5 = prim;
				}
				else
				{
					prim5 = 0.0;
				}
				P += prim;
				P5 += prim5;
			}
			inf = own_papers[i - 1].AIS / own_papers[i - 1].NAeff;
			if ((own_papers[i - 1].year <= an_FIN5) && (own_papers[i - 1].year >= an_INI5))
			{
				inf5 = inf;
			}
			else
			{
				inf5 = 0.0;
			}
			I += inf;
			I5 += inf5;
		}

		CiteFoundmine[i - 1] = 0;
		no_citari_valide = 0;
		no_citari_valide5 = 0;
		SCRIE = 0;
		OBS = 0;
		for (j = 0; j < contor_cites_general - 1; j++)
		{
			if ((others_papers[refs_individ[j].index_paper - 1].year < an_INI) || (others_papers[refs_individ[j].index_paper - 1].year > an_FIN)) // eliminare citare pentru limita de ani
			{
				continue;
			}

			if ((refs_individ[j].DOI != "") && (refs_individ[j].DOI == own_papers[i - 1].DOI)) // gasite cu DOI
			{
				CiteFoundmine[i - 1]++;
				SCRIE = 1;
				OBS = 0;
			}
			else
			{
				if ((refs_individ[j].DOI == ""))											// nu are DOI
				{
					if ((refs_individ[j].first_author == own_papers[i - 1].first_author) &&
						(refs_individ[j].journal == own_papers[i - 1].journal) &&
						(refs_individ[j].year == own_papers[i - 1].year))						// aceleasi
					{
						if ((refs_individ[j].s_pag != "") && (own_papers[i - 1].s_pag_start != "") && (refs_individ[j].s_pag == own_papers[i - 1].s_pag_start))
						{
							CiteFoundmine[i - 1]++;
							SCRIE = 1;
							OBS = 1;
						}
						else
						{
							if ((refs_individ[j].s_pag != "") && (own_papers[i - 1].s_art_no != "") && (refs_individ[j].s_pag == own_papers[i - 1].s_art_no))
							{
								CiteFoundmine[i - 1]++;
								SCRIE = 1;
								OBS = 1;
							}
						}
					}
				}
			}

			if (SCRIE)
			{
				//fprintf(fp_reviste_cite, "%s\n", others_papers[refs_individ[j].index_paper - 1].journal.c_str());

				if (others_papers[refs_individ[j].index_paper - 1].AIS >= AIS_LIMITA)
				{
					fprintf(fp_out_total, "\t\t[%d.%d] %s, %s, %s, vol. %s",
						i, CiteFoundmine[i - 1],
						others_papers[refs_individ[j].index_paper - 1].name.c_str(), others_papers[refs_individ[j].index_paper - 1].title.c_str(), others_papers[refs_individ[j].index_paper - 1].journal.c_str(), others_papers[refs_individ[j].index_paper - 1].s_vol.c_str());

					if ((others_papers[refs_individ[j].index_paper - 1].year <= an_FIN5) && (others_papers[refs_individ[j].index_paper - 1].year >= an_INI5))
					{
						fprintf(fp_out_total5, "\t\t[%d.%d] %s, %s, %s, vol. %s",
							i, CiteFoundmine[i - 1],
							others_papers[refs_individ[j].index_paper - 1].name.c_str(), others_papers[refs_individ[j].index_paper - 1].title.c_str(), others_papers[refs_individ[j].index_paper - 1].journal.c_str(), others_papers[refs_individ[j].index_paper - 1].s_vol.c_str());
					}
				}
				else
				{
					fprintf(fp_out_total, "\t\t*[%d.%d]* %s, %s, %s, vol. %s",
						i, CiteFoundmine[i - 1],
						others_papers[refs_individ[j].index_paper - 1].name.c_str(), others_papers[refs_individ[j].index_paper - 1].title.c_str(), others_papers[refs_individ[j].index_paper - 1].journal.c_str(), others_papers[refs_individ[j].index_paper - 1].s_vol.c_str());
					if ((others_papers[refs_individ[j].index_paper - 1].year <= an_FIN5) && (others_papers[refs_individ[j].index_paper - 1].year >= an_INI5))
					{
						fprintf(fp_out_total5, "\t\t*[%d.%d]* %s, %s, %s, vol. %s",
							i, CiteFoundmine[i - 1],
							others_papers[refs_individ[j].index_paper - 1].name.c_str(), others_papers[refs_individ[j].index_paper - 1].title.c_str(), others_papers[refs_individ[j].index_paper - 1].journal.c_str(), others_papers[refs_individ[j].index_paper - 1].s_vol.c_str());
					}
				}

				if (strcmp(others_papers[refs_individ[j].index_paper - 1].s_issue.c_str(), "")) //daca exista
				{
					fprintf(fp_out_total, "(%s), ", others_papers[refs_individ[j].index_paper - 1].s_issue.c_str());
					if ((others_papers[refs_individ[j].index_paper - 1].year <= an_FIN5) && (others_papers[refs_individ[j].index_paper - 1].year >= an_INI5))
					{
						fprintf(fp_out_total5, "(%s), ", others_papers[refs_individ[j].index_paper - 1].s_issue.c_str());
					}
				}
				else
				{
					fprintf(fp_out_total, ", ");
					if ((others_papers[refs_individ[j].index_paper - 1].year <= an_FIN5) && (others_papers[refs_individ[j].index_paper - 1].year >= an_INI5))
					{
						fprintf(fp_out_total5, ", ");
					}
				}
				if (strcmp(others_papers[refs_individ[j].index_paper - 1].s_pag_start.c_str(), "")) //daca exista
				{
					fprintf(fp_out_total, "pp. %s", others_papers[refs_individ[j].index_paper - 1].s_pag_start.c_str());
					if ((others_papers[refs_individ[j].index_paper - 1].year <= an_FIN5) && (others_papers[refs_individ[j].index_paper - 1].year >= an_INI5))
					{
						fprintf(fp_out_total5, "pp. %s", others_papers[refs_individ[j].index_paper - 1].s_pag_start.c_str());
					}
					if (strcmp(others_papers[refs_individ[j].index_paper - 1].s_pag_end.c_str(), "")) //daca exista
					{
						fprintf(fp_out_total, "-%s, ", others_papers[refs_individ[j].index_paper - 1].s_pag_end.c_str());
						if ((others_papers[refs_individ[j].index_paper - 1].year <= an_FIN5) && (others_papers[refs_individ[j].index_paper - 1].year >= an_INI5))
						{
							fprintf(fp_out_total5, "-%s, ", others_papers[refs_individ[j].index_paper - 1].s_pag_end.c_str());
						}
					}
					else
					{
						fprintf(fp_out_total, ", ");
						if ((others_papers[refs_individ[j].index_paper - 1].year <= an_FIN5) && (others_papers[refs_individ[j].index_paper - 1].year >= an_INI5))
						{
							fprintf(fp_out_total5, ", ");
						}
					}
				}
				if (strcmp(others_papers[refs_individ[j].index_paper - 1].s_art_no.c_str(), "")) //daca exista
				{
					fprintf(fp_out_total, "art.no. %s, ", others_papers[refs_individ[j].index_paper - 1].s_art_no.c_str());
					if ((others_papers[refs_individ[j].index_paper - 1].year <= an_FIN5) && (others_papers[refs_individ[j].index_paper - 1].year >= an_INI5))
					{
						fprintf(fp_out_total5, "art.no. %s, ", others_papers[refs_individ[j].index_paper - 1].s_art_no.c_str());
					}
				}
				else
				{
					fprintf(fp_out_total, ", ");
					if ((others_papers[refs_individ[j].index_paper - 1].year <= an_FIN5) && (others_papers[refs_individ[j].index_paper - 1].year >= an_INI5))
					{
						fprintf(fp_out_total5, ", ");
					}
				}
				if (OBS)
				{
					fprintf(fp_out_total, "(%d) %s\n", others_papers[refs_individ[j].index_paper - 1].year, others_papers[refs_individ[j].index_paper - 1].DOI.c_str());
					if ((others_papers[refs_individ[j].index_paper - 1].year <= an_FIN5) && (others_papers[refs_individ[j].index_paper - 1].year >= an_INI5))
					{
						fprintf(fp_out_total5, "(%d) %s\n", others_papers[refs_individ[j].index_paper - 1].year, others_papers[refs_individ[j].index_paper - 1].DOI.c_str());
					}
				}
				else
				{
					fprintf(fp_out_total, "(%d) %s\n", others_papers[refs_individ[j].index_paper - 1].year, others_papers[refs_individ[j].index_paper - 1].DOI.c_str());
					if ((others_papers[refs_individ[j].index_paper - 1].year <= an_FIN5) && (others_papers[refs_individ[j].index_paper - 1].year >= an_INI5))
					{
						fprintf(fp_out_total5, "(%d) %s\n", others_papers[refs_individ[j].index_paper - 1].year, others_papers[refs_individ[j].index_paper - 1].DOI.c_str());
					}
				}
				SCRIE = 0;
				OBS = 0;

				if (others_papers[refs_individ[j].index_paper - 1].AIS >= AIS_LIMITA)
				{
					cit += 1.0 / own_papers[i - 1].NAeff;
					no_citari_valide++;
					if ((others_papers[refs_individ[j].index_paper - 1].year <= an_FIN5) && (others_papers[refs_individ[j].index_paper - 1].year >= an_INI5))
					{
						cit5 += 1.0 / own_papers[i - 1].NAeff;
						no_citari_valide5++;
					}
				}
			}
		}
		C += cit;
		C5 += cit5;
		fprintf(fp_out_total, " autori=%d \t autori_eff=%5.3lf \t AIS=%5.3lf \t prim=%5.3lf \t inf=%5.3lf \t citari/valide=%d / %d \t cit=%5.3lf\n",
			own_papers[i - 1].NA, own_papers[i - 1].NAeff, own_papers[i - 1].AIS, prim, inf, CiteFoundmine[i - 1], no_citari_valide, cit);
		fprintf(fp_out_total, "-----------------------------------------------------------------------\n");

		fprintf(fp_out_total5, " autori=%d \t autori_eff=%5.3lf \t AIS=%5.3lf \t prim=%5.3lf \t inf=%5.3lf \t citari/valide=%d / %d \t cit=%5.3lf\n",
			own_papers[i - 1].NA, own_papers[i - 1].NAeff, own_papers[i - 1].AIS, prim5, inf5, CiteFoundmine[i - 1], no_citari_valide5, cit5);
		fprintf(fp_out_total5, "-----------------------------------------------------------------------\n");
	}
	fprintf(fp_out_total, "**********************************************************************************************************************\n");
	fprintf(fp_out_total, "P=%5.3lf \t\t\t I=%5.3lf \t\t\t C=%5.3lf\n", P, I, C);
	fprintf(fp_out_total, "**********************************************************************************************************************\n");
	fclose(fp_out_total);
	fprintf(fp_out_total5, "**********************************************************************************************************************\n");
	fprintf(fp_out_total5, "P=%5.3lf \t\t\t I=%5.3lf \t\t\t C=%5.3lf\n", P5, I5, C5);
	fprintf(fp_out_total5, "**********************************************************************************************************************\n");
	fclose(fp_out_total5);
	//fclose(fp_reviste_cite);
	//fclose(fp_reviste_mine);

	//////////////////////////////////////////////////////////////////////////////////////////////////UAIC 2013 - 2014
	double loc_uaic_1, loc_uaic_2;
	// 	int year1 = 2014;
	// 	int year2 = 2018;  // year2 >= year1

	fopen_s(&fp_out_total, path_OUT_UAIC, "w");
	//fprintf(fp_out_total, "ID1 \t NAME \t TITLE \t JOURNAL \t VOLUME \t ISSUE \t PAGE_START \t PAGE_END \t ART_NO \t YEAR \t DOI\n");
	for (i = 1; i < (Nmine - contor_lucrari_eliminate_limita_ani); i++)
	{
		loc_uaic_1 = 0;
		loc_uaic_2 = 0;
		fprintf(fp_out_total, "[%d] \t %s, %s, %s, vol. %s", i, own_papers[i - 1].name.c_str(), own_papers[i - 1].title.c_str(), own_papers[i - 1].journal.c_str(), own_papers[i - 1].s_vol.c_str());
		if (strcmp(own_papers[i - 1].s_issue.c_str(), "")) //daca exista
		{
			fprintf(fp_out_total, "(%s), ", own_papers[i - 1].s_issue.c_str());
		}
		else
		{
			fprintf(fp_out_total, ", ");
		}
		if (strcmp(own_papers[i - 1].s_pag_start.c_str(), "")) //daca exista
		{
			fprintf(fp_out_total, "pp. %s", own_papers[i - 1].s_pag_start.c_str());
			if (strcmp(own_papers[i - 1].s_pag_end.c_str(), "")) //daca exista
			{
				fprintf(fp_out_total, "-%s, ", own_papers[i - 1].s_pag_end.c_str());
			}
			else
			{
				fprintf(fp_out_total, ", ");
			}
		}
		if (strcmp(own_papers[i - 1].s_art_no.c_str(), "")) //daca exista
		{
			fprintf(fp_out_total, "art.no. %s, ", own_papers[i - 1].s_art_no.c_str());
		}

		fprintf(fp_out_total, "(%d) %s\n", own_papers[i - 1].year, own_papers[i - 1].DOI.c_str());

		if ((own_papers[i - 1].year >= year1) && (own_papers[i - 1].year <= year2))
		{
			loc_uaic_1 = (60.0 * own_papers[i - 1].IF + 25.0) / own_papers[i - 1].NA;
			UAIC_1 += loc_uaic_1;
		}
		UAIC_0 += own_papers[i - 1].IF / own_papers[i - 1].NA;

		CiteFoundmine[i - 1] = 0;
		no_citari_valide = 0;
		SCRIE = 0;
		OBS = 0;
		for (j = 0; j < contor_cites_general - 1; j++)
		{
			if ((refs_individ[j].DOI != "") && (refs_individ[j].DOI == own_papers[i - 1].DOI) && (others_papers[refs_individ[j].index_paper - 1].year >= year1 && others_papers[refs_individ[j].index_paper - 1].year <= year2)) // gasite cu DOI
			{
				CiteFoundmine[i - 1]++;
				SCRIE = 1;
				OBS = 0;
			}
			else
			{
				if ((refs_individ[j].DOI == "") && (others_papers[refs_individ[j].index_paper - 1].year >= year1 && others_papers[refs_individ[j].index_paper - 1].year <= year2))											// nu are DOI
				{
					if ((refs_individ[j].first_author == own_papers[i - 1].first_author) &&
						(refs_individ[j].journal == own_papers[i - 1].journal) &&
						(refs_individ[j].year == own_papers[i - 1].year))						// aceleasi
					{
						if ((refs_individ[j].s_pag != "") && (own_papers[i - 1].s_pag_start != "") && (refs_individ[j].s_pag == own_papers[i - 1].s_pag_start))
						{
							CiteFoundmine[i - 1]++;
							SCRIE = 1;
							OBS = 1;
						}
						else
						{
							if ((refs_individ[j].s_pag != "") && (own_papers[i - 1].s_art_no != "") && (refs_individ[j].s_pag == own_papers[i - 1].s_art_no))
							{
								CiteFoundmine[i - 1]++;
								SCRIE = 1;
								OBS = 1;
							}
						}
					}
				}
			}

			if (SCRIE)
			{
				fprintf(fp_out_total, "\t\t[%d.%d] %s, %s, %s, vol. %s",
					i, CiteFoundmine[i - 1],
					others_papers[refs_individ[j].index_paper - 1].name.c_str(), others_papers[refs_individ[j].index_paper - 1].title.c_str(), others_papers[refs_individ[j].index_paper - 1].journal.c_str(), others_papers[refs_individ[j].index_paper - 1].s_vol.c_str());

				if (strcmp(others_papers[refs_individ[j].index_paper - 1].s_issue.c_str(), "")) //daca exista
				{
					fprintf(fp_out_total, "(%s), ", others_papers[refs_individ[j].index_paper - 1].s_issue.c_str());
				}
				else
				{
					fprintf(fp_out_total, ", ");
				}
				if (strcmp(others_papers[refs_individ[j].index_paper - 1].s_pag_start.c_str(), "")) //daca exista
				{
					fprintf(fp_out_total, "pp. %s", others_papers[refs_individ[j].index_paper - 1].s_pag_start.c_str());
					if (strcmp(others_papers[refs_individ[j].index_paper - 1].s_pag_end.c_str(), "")) //daca exista
					{
						fprintf(fp_out_total, "-%s, ", others_papers[refs_individ[j].index_paper - 1].s_pag_end.c_str());
					}
					else
					{
						fprintf(fp_out_total, ", ");
					}
				}
				if (strcmp(others_papers[refs_individ[j].index_paper - 1].s_art_no.c_str(), "")) //daca exista
				{
					fprintf(fp_out_total, "art.no. %s, ", others_papers[refs_individ[j].index_paper - 1].s_art_no.c_str());
				}
				else
				{
					fprintf(fp_out_total, ", ");
				}
				if (OBS)
				{
					fprintf(fp_out_total, "(%d) %s\n", others_papers[refs_individ[j].index_paper - 1].year, others_papers[refs_individ[j].index_paper - 1].DOI.c_str());
				}
				else
				{
					fprintf(fp_out_total, "(%d) %s\n", others_papers[refs_individ[j].index_paper - 1].year, others_papers[refs_individ[j].index_paper - 1].DOI.c_str());
				}
				SCRIE = 0;
				OBS = 0;

				loc_uaic_2 += (20.0 * others_papers[refs_individ[j].index_paper - 1].IF + 10.0) / own_papers[i - 1].NA;

				fprintf(fp_out_total, "\t\t\t\t\tIF CITARE: %5.3lf\n", others_papers[refs_individ[j].index_paper - 1].IF);

				no_citari_valide++;
			}
		}

		C += cit;
		UAIC_2 += loc_uaic_2;
		fprintf(fp_out_total, " autori=%d \t \t IF=%5.3lf \t \t UAIC I.1(ISI)=%5.3lf \t nr.citari=%d \t UAIC I.12 (Citari)=%5.3lf\n",
			own_papers[i - 1].NA, own_papers[i - 1].IF, loc_uaic_1, CiteFoundmine[i - 1], loc_uaic_2);
		fprintf(fp_out_total, "-----------------------------------------------------------------------\n");
	}
	fprintf(fp_out_total, "**********************************************************************************************************************\n");
	fprintf(fp_out_total, "T  O  T  A  L      \n", UAIC_1, UAIC_2);

	fprintf(fp_out_total, "**********************************************************************************************************************\n");
	fprintf(fp_out_total, "UAIC I.1 (ISI) = %5.3lf \t\t\t UAIC I.12 (Citari) = %5.3lf \n", UAIC_1, UAIC_2);
	fprintf(fp_out_total, "**********************************************************************************************************************\n");
	fprintf(fp_out_total, "IMPACT FACTOR TOTAL (TOTI ANII) = %5.3lf  \n", UAIC_0);
	fprintf(fp_out_total, "**********************************************************************************************************************\n");
	fclose(fp_out_total);

	//////////////////////////////////////////////////////////////////////////////////////////////////P,I,C Abilitare 2013

	char pentru_citari[250];
	fopen_s(&fp_out_total, path_OUT_IP, "w");
	for (i = 1; i < (Nmine - contor_lucrari_eliminate_limita_ani); i++)
	{
		//if (own_papers[i - 1].AIS < AIS_LIMITA)
		//{
		//	continue;
		//}

		own_papers[i - 1].title.erase(
			remove(own_papers[i - 1].title.begin(), own_papers[i - 1].title.end(), '\"'),
			own_papers[i - 1].title.end()
		);

		fprintf(fp_out_total, "%d \t \"%s\" \t \"%s\" \t \"%s\" \t %s \t ", i, own_papers[i - 1].name.c_str(), own_papers[i - 1].title.c_str(), own_papers[i - 1].journal.c_str(), own_papers[i - 1].s_vol.c_str());

		// 		if (strcmp(own_papers[i - 1].s_issue.c_str(), "")) //daca exista
		// 		{
		// 			fprintf(fp_out_total, "(%s), ", own_papers[i - 1].s_issue.c_str());
		// 		}
		// 		else
		// 		{
		// 			fprintf(fp_out_total, ", ");
		// 		}
		if (strcmp(own_papers[i - 1].s_pag_start.c_str(), "")) //daca exista
		{
			fprintf(fp_out_total, "%s \t ", own_papers[i - 1].s_pag_start.c_str());
		}
		else
		{
			if (strcmp(own_papers[i - 1].s_pag_end.c_str(), "")) //daca exista
			{
				fprintf(fp_out_total, "%s \t ", own_papers[i - 1].s_pag_end.c_str());
			}
		}
		if (strcmp(own_papers[i - 1].s_art_no.c_str(), "")) //daca exista
		{
			fprintf(fp_out_total, "%s \t ", own_papers[i - 1].s_art_no.c_str());
		}

		fprintf(fp_out_total, "%d \t ", own_papers[i - 1].year);

		if (own_papers[i - 1].AIS >= AIS_LIMITA)
		{
			if (own_papers[i - 1].prim)
			{
				fprintf(fp_out_total, "1 \t  %5.3lf  \t %5.3lf  ", own_papers[i - 1].AIS, own_papers[i - 1].AIS);
			}
			else
			{
				fprintf(fp_out_total, "0 \t  0  \t %5.3lf  ", own_papers[i - 1].AIS);
			}

			fprintf(fp_out_total, "\t %d \t %5.3lf \t %5.3lf \t %5.3lf\n", own_papers[i - 1].NA, own_papers[i - 1].NAeff, own_papers[i - 1].AIS / own_papers[i - 1].NAeff, own_papers[i - 1].IF);
		}
		else
		{
			if (own_papers[i - 1].prim)
			{
				fprintf(fp_out_total, "1 \t  %5.3lf  \t %5.3lf  ", own_papers[i - 1].AIS, own_papers[i - 1].AIS);
			}
			else
			{
				fprintf(fp_out_total, "0 \t  0  \t %5.3lf  ", own_papers[i - 1].AIS);
			}

			fprintf(fp_out_total, "\t %d \t %5.3lf \t %5.3lf \t %5.3lf\n", own_papers[i - 1].NA, own_papers[i - 1].NAeff, 0.0, own_papers[i - 1].IF);
		}
	}
	fclose(fp_out_total);

	fopen_s(&fp_out_total, path_OUT_C, "w");
	for (i = 1; i < (Nmine - contor_lucrari_eliminate_limita_ani); i++)
	{
		////////////////////////////////////////////////////////////////////////// NUMAI NUMARARE
		CiteFoundmine[i - 1] = 0;
		no_citari_valide = 0;
		SCRIE = 0;
		OBS = 0;
		for (j = 0; j < contor_cites_general - 1; j++)
		{
			if ((others_papers[refs_individ[j].index_paper - 1].year < an_INI) || (others_papers[refs_individ[j].index_paper - 1].year > an_FIN))
			{
				continue;
			}

			if (others_papers[refs_individ[j].index_paper - 1].AIS < AIS_LIMITA)
			{
				continue;
			}

			if ((refs_individ[j].DOI != "") && (refs_individ[j].DOI == own_papers[i - 1].DOI)) // gasite cu DOI
			{
				CiteFoundmine[i - 1]++;
				SCRIE = 1;
				OBS = 0;
			}
			else
			{
				if ((refs_individ[j].DOI == ""))											// nu are DOI
				{
					if ((refs_individ[j].first_author == own_papers[i - 1].first_author) &&
						(refs_individ[j].journal == own_papers[i - 1].journal) &&
						(refs_individ[j].year == own_papers[i - 1].year))						// aceleasi
					{
						if ((refs_individ[j].s_pag != "") && (own_papers[i - 1].s_pag_start != "") && (refs_individ[j].s_pag == own_papers[i - 1].s_pag_start))
						{
							CiteFoundmine[i - 1]++;
							SCRIE = 1;
							OBS = 1;
						}
						else
						{
							if ((refs_individ[j].s_pag != "") && (own_papers[i - 1].s_art_no != "") && (refs_individ[j].s_pag == own_papers[i - 1].s_art_no))
							{
								CiteFoundmine[i - 1]++;
								SCRIE = 1;
								OBS = 1;
							}
						}
					}
				}
			}
		}
		////////////////////////////////////////////////////////////////////////// END NUMARARE

		if (CiteFoundmine[i - 1] == 0)
		{
			continue;
		}

		own_papers[i - 1].title.erase(
			remove(own_papers[i - 1].title.begin(), own_papers[i - 1].title.end(), '\"'),
			own_papers[i - 1].title.end()
		);
		own_papers[i - 1].title.erase(
			remove(own_papers[i - 1].title.begin(), own_papers[i - 1].title.end(), '\''),
			own_papers[i - 1].title.end()
		);

		fprintf(fp_out_total, "\" \" \t \"%s\" \t \"%s\" \t \"%s\" \t %s \t ", own_papers[i - 1].name.c_str(), own_papers[i - 1].title.c_str(), own_papers[i - 1].journal.c_str(), own_papers[i - 1].s_vol.c_str());
		if (strcmp(own_papers[i - 1].s_pag_start.c_str(), "")) //daca exista
		{
			fprintf(fp_out_total, "%s \t ", own_papers[i - 1].s_pag_start.c_str());
		}
		else
		{
			if (strcmp(own_papers[i - 1].s_pag_end.c_str(), "")) //daca exista
			{
				fprintf(fp_out_total, "%s \t ", own_papers[i - 1].s_pag_end.c_str());
			}
		}
		if (strcmp(own_papers[i - 1].s_art_no.c_str(), "")) //daca exista
		{
			fprintf(fp_out_total, "%s \t ", own_papers[i - 1].s_art_no.c_str());
		}

		fprintf(fp_out_total, "%d \t \" \" \t ", own_papers[i - 1].year);

		fprintf(fp_out_total, "%d \t %5.3lf \t=%5.3lf/%5.3lf\n", own_papers[i - 1].NA, own_papers[i - 1].NAeff, (double)CiteFoundmine[i - 1], own_papers[i - 1].NAeff);

		////////////////////////////////////////////////////////////////////////// IAR NUMARARE PENTRU SCRIERE
		CiteFoundmine[i - 1] = 0;
		no_citari_valide = 0;
		SCRIE = 0;
		OBS = 0;
		for (j = 0; j < contor_cites_general - 1; j++)
		{
			if (others_papers[refs_individ[j].index_paper - 1].AIS < AIS_LIMITA)
			{
				continue;
			}

			if ((refs_individ[j].DOI != "") && (refs_individ[j].DOI == own_papers[i - 1].DOI)) // gasite cu DOI
			{
				CiteFoundmine[i - 1]++;
				SCRIE = 1;
				OBS = 0;
			}
			else
			{
				if ((refs_individ[j].DOI == ""))											// nu are DOI
				{
					if ((refs_individ[j].first_author == own_papers[i - 1].first_author) &&
						(refs_individ[j].journal == own_papers[i - 1].journal) &&
						(refs_individ[j].year == own_papers[i - 1].year))						// aceleasi
					{
						if ((refs_individ[j].s_pag != "") && (own_papers[i - 1].s_pag_start != "") && (refs_individ[j].s_pag == own_papers[i - 1].s_pag_start))
						{
							CiteFoundmine[i - 1]++;
							SCRIE = 1;
							OBS = 1;
						}
						else
						{
							if ((refs_individ[j].s_pag != "") && (own_papers[i - 1].s_art_no != "") && (refs_individ[j].s_pag == own_papers[i - 1].s_art_no))
							{
								CiteFoundmine[i - 1]++;
								SCRIE = 1;
								OBS = 1;
							}
						}
					}
				}
			}
			////////////////////////////////////////////////////////////////////////// END NUMARARE PENTRU SCRIERE

			if (SCRIE)
			{
				fprintf(fp_out_total, "%d \t \"%s\" \t \"%s\" \t \"%s\" \t \"%s\" \t ",
					CiteFoundmine[i - 1],
					others_papers[refs_individ[j].index_paper - 1].name.c_str(), others_papers[refs_individ[j].index_paper - 1].title.c_str(), others_papers[refs_individ[j].index_paper - 1].journal.c_str(), others_papers[refs_individ[j].index_paper - 1].s_vol.c_str());

				if (strcmp(others_papers[refs_individ[j].index_paper - 1].s_pag_start.c_str(), "")) //daca exista
				{
					fprintf(fp_out_total, "%s \t ", others_papers[refs_individ[j].index_paper - 1].s_pag_start.c_str());
				}
				else
				{
					if (strcmp(others_papers[refs_individ[j].index_paper - 1].s_pag_end.c_str(), "")) //daca exista
					{
						fprintf(fp_out_total, "%s \t ", others_papers[refs_individ[j].index_paper - 1].s_pag_end.c_str());
					}
				}
				if (strcmp(others_papers[refs_individ[j].index_paper - 1].s_art_no.c_str(), "")) //daca exista
				{
					fprintf(fp_out_total, "%s \t ", others_papers[refs_individ[j].index_paper - 1].s_art_no.c_str());
				}

				//if (OBS)
				//{
				fprintf(fp_out_total, "%d \t ", others_papers[refs_individ[j].index_paper - 1].year);
				//}

				SCRIE = 0;
				OBS = 0;

				if (others_papers[refs_individ[j].index_paper - 1].AIS >= AIS_LIMITA)
				{
					fprintf(fp_out_total, "%5.3lf \t %5.3lf \t \" \" \t \" \"\n", others_papers[refs_individ[j].index_paper - 1].AIS, others_papers[refs_individ[j].index_paper - 1].IF);
				}
			}
		}
		//sprintf(pentru_citari, "%d / %5.3lf", CiteFoundmine[i - 1], own_papers[i - 1].NAeff);
//		fprintf(fp_out_total, " , , , %s, =,%5.3lf\n", pentru_citari, CiteFoundmine[i - 1] / own_papers[i - 1].NAeff);
	}
	fclose(fp_out_total);

	return 0;
}