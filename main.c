#include <stdio.h>
#include <stdlib.h>

#define BROJ_BAKETA 7
#define FIKSNI_KORAK 1
#define FAKTOR_BAKETIRANJA 3
#define MAX_IME_DAT 32

FILE *pomocna;
FILE *aktivna;
FILE *greske;
char ime_aktivne_datoteke[MAX_IME_DAT];

enum status_sloga{PRAZAN=0,AKTIVAN,NEAKTIVAN};

typedef struct
{
    unsigned long evid_broj;
    char datum_vreme[21];
    char broj_racuna[32];
    char opis_svrhe[120];
    unsigned long iznos;
    enum status_sloga status;
} Slog;

// Struktura baketa
typedef struct
{
	Slog slogovi[FAKTOR_BAKETIRANJA];

} Baket;

unsigned int TransformID(unsigned long key) {
    return key%BROJ_BAKETA + 1;
}

void UpisiBaket(FILE *f, char *ime_dat, unsigned int adresa, Baket* baket) {

     if(adresa < 1 || adresa > BROJ_BAKETA) {
    	fprintf("Pogresna adresa baketa %d\n", adresa);
		return;
    }

    if(fseek(f, (adresa-1)*sizeof(Baket), SEEK_SET) != 0) {
        fprintf("Greska pri postavljanju pokazivaca datoteke <%s>\n", "imedat");
		return;
    }

    if(fwrite(baket, sizeof(Baket), 1, f) != 1)
        fprintf("Greska pri pisanju baketa u datoteku <%s>\n", ime_dat);
}

void FormiranjeDatoteke() {
	FILE *nova_datoteka;
	char ime_dat[MAX_IME_DAT];
	int i;

	printf("Unesite naziv datoteke: ");
	fflush(stdin);
	gets(ime_dat);

	if((nova_datoteka = fopen(ime_dat, "wb"))== NULL) {
		printf("\nGreska pri otvaranju datoteke <%s>", ime_dat);
		return;
	}
	else {
		Baket baket;
		for(i=0; i<FAKTOR_BAKETIRANJA; i++){
			baket.slogovi[i].evid_broj=0;
			baket.slogovi[i].broj_racuna[0]='\0';
			baket.slogovi[i].datum_vreme[0]='\0';
			baket.slogovi[i].opis_svrhe[0]='\0';
			baket.slogovi[i].iznos=0;
			baket.slogovi[i].status = PRAZAN;
		}
		for(i=1; i<=BROJ_BAKETA; i++)
			UpisiBaket(nova_datoteka, ime_dat, i, &baket);

		printf("\nDatoteka <%s> je uspesno kreirana", ime_dat);
	}

	fclose(nova_datoteka);

}

void IzborAktivneDatoteke() {
    FILE *nova_aktivna;
    char ime_dat[MAX_IME_DAT];

	printf("Unos imena nove aktivne datoteke: ");
	fflush(stdin);
	gets(ime_dat);

	if((nova_aktivna = fopen(ime_dat, "rb+"))==NULL) {
		printf("\nGreska pri otvaranju datoteke sa imenom <%s>", ime_dat);
		return;
	}
	else {
		if(aktivna != NULL)
			fclose(aktivna);

		aktivna = nova_aktivna;
		strcpy(ime_aktivne_datoteke, ime_dat);

		printf("\nDatoteka <%s> je postala aktivna", ime_dat);
	}

}

void PrikazAktivne() {

	if(aktivna == NULL) {
		printf("Trenutno ne postoji aktivna datoteka\n");
	}
	else {
		printf("Aktivna datoteka je <%s>", ime_aktivne_datoteke);
	}
}
void UpisSloga() {
    Slog slog;

    printf("Unos sloga:\n");

    do{
        printf("\nUnesite evidencioni broj transakcije [najvise 9 cifara]: ");
        fflush(stdin);
        scanf("%d", &slog.evid_broj);
    }while(slog.evid_broj<1 || slog.evid_broj>999999999);

    do{
        printf("\nUnesite broj racuna [najvise 32 karaktera]: ");
        fflush(stdin);
        scanf("%s", &slog.broj_racuna);
    }while(strlen(slog.broj_racuna)>32);

    do{
        printf("\nUnesite datum i vreme transakcije [najvise 20 karaktera]: ");
        fflush(stdin);
        gets(slog.datum_vreme);
    }while(strlen(slog.datum_vreme)>20);

    do{
        printf("\nUnesite opis svrhe transakcije [do 120 karaktera]: ");
        fflush(stdin);
        scanf("%s", &slog.opis_svrhe);
    }while(strlen(slog.opis_svrhe)>120);

    do{
        printf("\nUnesite iznos [najvise 1 000 000 000]: ");
        fflush(stdin);
        scanf("%d", &slog.iznos);
    }while(slog.iznos>1000000000 || slog.iznos<1);

    slog.status=NEAKTIVAN;

    if((fwrite(&slog, sizeof(Slog), 1, pomocna))!=1){
        puts("Greska prilikom punjenja pomocne datoteke!");
        return;
    }
    puts("\nSlog je uspesno unet u pomocnu datoteku!");

}

void UcitajBaket(FILE *f, char *ime_dat, unsigned int adresa, Baket* baket) {

     if(adresa < 1 || adresa > BROJ_BAKETA) {
    	fprintf("Pogresna adresa baketa %d\n", adresa);
		return;
    }

    if(fseek(f, (adresa-1)*sizeof(Baket), SEEK_SET) != 0) {
        fprintf("Greska pri postavljanju pokazivaca datoteke <%s>\n", "imedat");
		return;
    }

    if(fread(baket, sizeof(Baket), 1, f) != 1)
        fprintf("Greska pri citanju baketa iz datoteke <%s>\n", ime_dat);
}


void FormiranjeAktivneDatoteke(){
    int i, j, slog_upisan, brojNovihSlogova;
    unsigned int maticniBaket, trenutniBaket;
    Slog slog;
    Baket baket;

    if(aktivna == NULL) {
		printf("Nije izabrana aktivna datoteka\n");
        return;
	}

    fseek(pomocna, 0L, SEEK_END);
    brojNovihSlogova = ftell(pomocna)/sizeof(Slog);
    fseek(pomocna, 0L, SEEK_SET);

    for(i=0; i < brojNovihSlogova; i++){
        if(fread(&slog, sizeof(Slog), 1, pomocna)!=1){
            printf("%d  Greska prilikom citanja pomocne datoteke.",i);
            return;
        }
        slog_upisan=0;

        maticniBaket = trenutniBaket = TransformID(slog.evid_broj);

        do{
            UcitajBaket(aktivna, ime_aktivne_datoteke, trenutniBaket, &baket);
            for(j=0; j < FAKTOR_BAKETIRANJA; j++){
                if(baket.slogovi[j].evid_broj == slog.evid_broj && baket.slogovi[j].status == AKTIVAN){
                    printf("Slog sa identifikacionim brojem %d vec postoji u aktivnoj datoteci!\n",slog.evid_broj);
                    slog_upisan = 1;
                    break;
                }
            }
            trenutniBaket += FIKSNI_KORAK;
            if(trenutniBaket > BROJ_BAKETA)
                trenutniBaket = trenutniBaket%BROJ_BAKETA;

            }while(!slog_upisan && trenutniBaket != maticniBaket);
            if(slog_upisan)
                continue;
        do{
            UcitajBaket(aktivna, ime_aktivne_datoteke, trenutniBaket, &baket);
            for(j=0; j < FAKTOR_BAKETIRANJA; j++){
                if(baket.slogovi[j].status == PRAZAN){
                    slog.status = AKTIVAN;
                    baket.slogovi[j] = slog;
                    UpisiBaket(aktivna, ime_aktivne_datoteke, trenutniBaket, &baket);
                    slog_upisan = 1;
                    break;
                }
            }
            trenutniBaket += FIKSNI_KORAK;
            if(trenutniBaket > BROJ_BAKETA)
                trenutniBaket = trenutniBaket%BROJ_BAKETA;

            }while(!slog_upisan && trenutniBaket != maticniBaket);
    }
    if((pomocna=fopen("Pomocna.bin", "wb+")) == NULL){
        printf("\nNeuspesno otvaranje pomocne datoteke!");
        exit(1);
    }
    printf("\nAktivna datoteka je uspesno azurirana!");
}

void PrikazSvihSlogova(){
    Baket baket;
    int i,j;

    if(aktivna == NULL) {
		printf("Nije izabrana aktivna datoteka\n");
        return;
	}

    for(i=1; i <= BROJ_BAKETA; i++){
        UcitajBaket(aktivna, ime_aktivne_datoteke, i, &baket);
        for(j=0; j<FAKTOR_BAKETIRANJA; j++){
            if(baket.slogovi[j].status == AKTIVAN){
                printf("\n== Baket: %d ==== Slog: %d =========",i,j+1);
                printf("\nEvidencioni broj transakcije: %d", baket.slogovi[j].evid_broj);
                printf("\nBroj racuna:            %s", baket.slogovi[j].broj_racuna);
                printf("\nOpis svrhe transakcije:    %s", baket.slogovi[j].opis_svrhe);
                printf("\nDatum i vreme transakcije:        %s", baket.slogovi[j].datum_vreme);
                printf("\nIznos:      %d", baket.slogovi[j].iznos);
                printf("\n==================================\n");
            }

        }
    }
}

void LogickoBrisanje(){
    unsigned int maticniBaket, trenutniBaket;;
    int slog_izbrisan = 0,j;
    unsigned long evBroj;
    Baket baket;

    if(aktivna == NULL) {
		printf("Nije izabrana aktivna datoteka\n");
        return;
	}

    printf("Unesite evidencioni broj sloga koji zelite da obrisete: ");
    scanf("%d", &evBroj);
    maticniBaket = trenutniBaket = TransformID(evBroj);
    do{
        UcitajBaket(aktivna, ime_aktivne_datoteke, trenutniBaket, &baket);
        for(j=0; j < FAKTOR_BAKETIRANJA; j++)
            if(baket.slogovi[j].evid_broj == evBroj){
               baket.slogovi[j].status = PRAZAN;
               UpisiBaket(aktivna, ime_aktivne_datoteke, trenutniBaket, &baket);
               slog_izbrisan = 1;
               break;
            }
         trenutniBaket += FIKSNI_KORAK;
         if(trenutniBaket > BROJ_BAKETA)
            trenutniBaket = trenutniBaket%BROJ_BAKETA;

    }while(!slog_izbrisan && trenutniBaket != maticniBaket);
    if(slog_izbrisan)
        printf("Slog je obrisan.");
    else
        printf("Ne postoji slog sa evidencionim brojem %d",evBroj);

}

void IzmenaDatuma(){
    unsigned int maticniBaket, trenutniBaket;;
    int slog_izmenjen = 0,j;
    unsigned long evBroj;
    Baket baket;
    char datum[21];

    if(aktivna == NULL) {
		printf("Nije izabrana aktivna datoteka\n");
        return;
	}

    printf("Unesite evidencioni broj sloga koji zelite da izmenite: ");
    scanf("%d", &evBroj);
    maticniBaket = trenutniBaket = TransformID(evBroj);
    do{
        UcitajBaket(aktivna, ime_aktivne_datoteke, trenutniBaket, &baket);
        for(j=0; j < FAKTOR_BAKETIRANJA; j++)
            if(baket.slogovi[j].evid_broj == evBroj && baket.slogovi[j].status == AKTIVAN){
                printf("\nSlog pronadjen:");
                printf("\n== Baket: %d ==== Slog: %d =========",trenutniBaket,j+1);
                printf("\nEvidencioni broj transakcije: %d", baket.slogovi[j].evid_broj);
                printf("\nBroj racuna:            %s", baket.slogovi[j].broj_racuna);
                printf("\nOpis svrhe transakcije:    %s", baket.slogovi[j].opis_svrhe);
                printf("\nDatum i vreme transakcije:        %s", baket.slogovi[j].datum_vreme);
                printf("\nIznos:      %d", baket.slogovi[j].iznos);
                printf("\n==================================\n");
                printf("\nUnesite novi datum i vreme transakcije: ");
                fflush(stdin);
                gets(datum);
                strcpy(baket.slogovi[j].datum_vreme,datum);
                UpisiBaket(aktivna, ime_aktivne_datoteke, trenutniBaket, &baket);
                slog_izmenjen = 1;
                break;
            }
         trenutniBaket += FIKSNI_KORAK;
         if(trenutniBaket > BROJ_BAKETA)
             trenutniBaket = trenutniBaket%BROJ_BAKETA;

    }while(!slog_izmenjen && trenutniBaket != maticniBaket);
    if(slog_izmenjen)
        printf("Datum i vreme transakcije su uspesno izmenjeni.");
    else
        printf("Ne postoji slog sa evidencionim brojem %d",evBroj);

}
void ZatvoriDatoteke() {

	if(aktivna != NULL) {
		fflush(aktivna);
		fclose(aktivna);
	}
	if(pomocna != NULL) {
		fflush(pomocna);
		fclose(pomocna);
	}
}

int main()
{
    int izbor;

	aktivna=NULL;
	strcpy(ime_aktivne_datoteke, "");
	if((pomocna=fopen("Pomocna.bin", "wb+")) == NULL) {
        printf("\nNeuspesno otvaranje pomocne datoteke!");
        return 1;
    }

    do {
        do {
            system("cls");
            puts("1.Formiranje nove datoteke");
            puts("2.Izbor aktivne datoteke");
            puts("3.Prikaz naziva aktivne datoteke");
            puts("4.Upis novog sloga");
            puts("5.Formiranje aktivne datoteka");
            puts("6.Prikaz svih slogova");
            puts("7.Brisanje aktuelnog sloga");
            puts("8.Izmena datuma i vremena sloga");
            puts("9.Kraj\n");
            printf("Vas izbor: ");

            fflush(stdin);
            izbor = getch() -'0';
            } while(izbor < 1 || izbor > 9);

            system("cls");
            switch(izbor) {
            case 1:	 FormiranjeDatoteke();              break;
            case 2:	 IzborAktivneDatoteke();            break;
            case 3:	 PrikazAktivne();	                break;
            case 4:	 UpisSloga();   		            break;
            case 5:  FormiranjeAktivneDatoteke();	    break;
            case 6:  PrikazSvihSlogova();      	        break;
			case 7:  LogickoBrisanje();	                break;
			case 8:  IzmenaDatuma();                    break;
            case 9:	 ZatvoriDatoteke();        	        break;
        }

        if(izbor!=9) {
            printf("\n\nPritisnite bilo koji taster za nastavak...");
            fflush(stdin);
            _getch();
        }
        system("cls");

    }   while(izbor != 9);

    return 0;
}
