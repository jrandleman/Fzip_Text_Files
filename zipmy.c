/* AUTHOR: JORDAN RANDLEMAN */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#define CH_IS_GNUM(ch_n_inst) ((ch_n_inst) >= '0' && (ch_n_inst) <= '9')
#define CH_IS_GCAP(ch_c_inst) ((ch_c_inst) >= 'A' && (ch_c_inst) <= 'Z')
/* 'HIDE' FILE FUNCTIONS */
void hide(char *, char *);
void write_bin_ss_keys(char [][151], int, char *);
void read_txt_engl(char *, char *);
void write_txt_compressed(char *);
/* 'SHOW' FILE FUNCTIONS */
void show(char *, char *);
void read_bin_ss_keys(char *);
void show_txt_compressed(char *, char *);
/* BOTH => ERROR/ENCRYPTION FUNCTIONS */
void err_info();
void delta_txt_crypt(char *, char *);
/* CHARACTER/COMPRESSION FUNCTIONS */
void rm_nxt_ch(char *, int);
void modify_s(char *, int);
void s_decompress(char [][151], char *);
int s_compress(char [][151], char *);
int process_split_s(char [][151], char *, char *, char *);
/* SUBSTRING REFERENCE KEY HANDLER FUNCTIONS */
int is_ss_ref(int, char);
int ss_ch_index(char);
char adjust_ref_char(char);
/* SUBSTRING FUNCTIONS */
void store_ss(char [][151], char *, int);
int clean_ss(char [][151], char *, int);
void splice_ss(char [][151], int [50], int, int);
void trim_ss(char [][151], char *, int [50], int, int);
void print_ss(char [][151], int);
/* GLOBAL VARIABLES */
char ss_array_matrix[300][50][151], s_compress_storage[300][151], s_max_buffer[30000];
char ss_refs[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$<=>@[]^{|}~\0", nchar = '\n', tchar = '\t';
int chunk_count = 0, original_bytes = 0, compressed_bytes = 0, zip_info = 0;

int main(int argc, char *argv[]) {
	if(argc < 4) {
		err_info();
		return 0;
	}
	char *action = argv[3], *show_info;
	modify_s(action, 0); /* lowify */
	if(argc == 5) {
		show_info = argv[4];
		modify_s(show_info, 0); /* lowify */
		if(strcmp(show_info, "info") == 0) zip_info = 1;
	}
	if(strcmp(action, "hide") == 0) {
		hide(argv[2], argv[1]);
	} else if(strcmp(action, "show") == 0) {
		show(argv[2], argv[1]);
	} else {
		err_info();
	}
	return 0;
}
/******************************************************************************
* HIDE / SHOW FUNCTIONS
******************************************************************************/
void hide(char *arg2, char *arg1) {
	read_txt_engl(arg2, arg1); /* process text file in 150 char chunks */
	write_txt_compressed(arg1); /* write compressed string to text file */
	/* output results */
	float bytes_saved = 100*(1.000000 - ((float)compressed_bytes)/((float)original_bytes));
	printf("\n>> %s => COMPRESSED AND ENCRYPTED!\n\n", arg1);
	printf(">> BYTES => UNCOMPRESSED: %d, COMPRESSED: %d, COMPRESSION RATE: %.2f%%\n\n", original_bytes, compressed_bytes, bytes_saved);
}
void show(char *arg2, char *arg1) {
	read_bin_ss_keys(arg2);
	show_txt_compressed(arg2, arg1);
	printf("\n>> %s => DECOMPRESSED AND DECRYPTED!\n\n", arg1);
}
void err_info() {
	printf("\n==================== INVALID EXECUTION! ====================\n");
	printf("$ gcc -o zipmy zipmy.c\n$ ./zipmy textfile.txt yourpassword hide/show\n");
	printf("============================================================");
	printf("\n=> FILE.TXT FORMAT: AVOID NUMBERS, NEWLINES, & UNDERSCORES!\n");
	printf("============================================================");
	printf("\n=> INFORMATION (DO W/ BOTH HIDE/SHOW): $ ... hide/show info\n");
	printf("============================================================\n\n");
}
/******************************************************************************
* TEXT FILE FUNCTIONS
******************************************************************************/
void show_txt_compressed(char *arg2, char *arg1) {
	FILE *fp;
	char s_buffer[150];
	int s_chunk_number = 0;
	if((fp = fopen(arg1, "r")) == NULL) {
		printf("(!!!) ERR READING COMPRESSED TEXT FILE (!!!)\n");
		fclose(fp);
		return;
	}
	while(fgets(s_buffer, 150, fp) > 0) { /* GET COMPRESSED TEXT */
		char *p = s_buffer;
		while(*(p + 1) != '\0') p++;
		*p = '\0'; /* remove \n from s_buffer */
		if(zip_info == 1) printf("\nENCRYPTED/COMPRESSED: %s\n", s_buffer);
		delta_txt_crypt(s_buffer, arg2); /* decrypt text */
		s_decompress(ss_array_matrix[s_chunk_number], s_buffer);
		modify_s(s_buffer, 2); /* '_' => ' ' */
		if(zip_info == 1) printf("DECRYPTED/DECOMPRESS: %s\n", s_buffer);
		strcpy(s_compress_storage[s_chunk_number], s_buffer);
		s_chunk_number++;
	}
	for(int i = 0; i < s_chunk_number; i++) {
		char *p = s_max_buffer;
		if(i != 0) while(*p != '\0') p++;
		sprintf(p, "%s", s_compress_storage[i]);
	}
	fclose(fp);

	if((fp = fopen(arg1, "w")) == NULL) { /* PUT DECOMPRESSED TEXT */
		printf("(!!!) ERR WRITING DECOMPRESSED TEXT FILE (!!!)\n");
		fclose(fp);
		return;
	}
	fwrite(s_max_buffer, sizeof(char), strlen(s_max_buffer), fp);
	fclose(fp);
	return;
}
void read_txt_engl(char *arg2, char *arg1) {
	char ss[50][151], s_chunk[151];
	int ret;
	FILE *fp;
	if((fp = fopen(arg1, "r")) == NULL) {
		printf("\n(!!!) ERR READING TEXT FILE (!!!)\n\n");
		fclose(fp);
		exit(0);
	}
	while((ret = fread(s_chunk, sizeof(char), 150, fp)) > 0) {
		s_chunk[ret] = '\0';
		original_bytes += ret;
		compressed_bytes += process_split_s(ss, s_chunk, s_compress_storage[chunk_count], arg2);
		chunk_count++;
	}
	fclose(fp);
}
void write_txt_compressed(char *arg1) {
	FILE *fp;
	if((fp = fopen(arg1, "w")) == NULL) {
		printf("(!!!) ERR WRITING COMPRESSED TEXT TO FILE (!!!)\n");
		fclose(fp);
		return;
	}
	for(int i = 0; i < chunk_count; i++) {
		fwrite(s_compress_storage[i], sizeof(char), strlen(s_compress_storage[i]), fp);
		fwrite(&nchar, sizeof(char), 1, fp);
	}
	fclose(fp);
}
/******************************************************************************
* BINARY FILE FUNCTIONS
******************************************************************************/
void read_bin_ss_keys(char *arg2) {
	char ss_buffer[50];
	int n = 0, m = 0; /* m - 2D ss array (sentence), n - ss (word) => WRT ss_array_matrix[m][n][] */
	FILE *fp;
	if((fp = fopen(arg2, "rb")) == NULL) {
		printf("\n(!!!) ERR PROCESSING PASSWORD (!!!)\n\n");
		fclose(fp);
		exit(0);
	}
	while(fread(ss_buffer, sizeof(char), 50, fp) == 50) {
		char *p = ss_buffer, *q = ss_array_matrix[m][n];
		while(*p != '\0') *q++ = *p++; /* copy ss from bin to 2d ss array in 3d matrix */
		*q = '\0';  /* terminate ss */
		if(ss_buffer[49] == '\n') {
			ss_array_matrix[m][n+1][0] = '\0'; /* terminate ss 2d array */
			m++; /* move to next 2d ss array (1 per s_chunk) in 3d matrix */
			n = 0; /* start from row (ss) 0 */
		} else {
			n++; /* move to next row (ss) */
		}
	}
	fclose(fp);
	remove(arg2); /* delete password binary file once ss keys retrieved */
}
void write_bin_ss_keys(char ss[][151], int ss_total, char *arg2) {
	FILE *fp;
	(chunk_count == 0) ? (fp = fopen(arg2, "wb")) : (fp = fopen(arg2, "ab"));
	if(fp == NULL) {
		printf("(!!!) ERR LINKING PASSWORD TO TEXT (!!!)\n");
		fclose(fp);
		return;
	}
	for(int i = 0; i < ss_total; i++) {
		fwrite(ss[i], sizeof(char), 49, fp);
		if(i < ss_total - 1) fwrite(&tchar, sizeof(char), 1, fp);
	}
	fwrite(&nchar, sizeof(char), 1, fp);
	fclose(fp);
}
/******************************************************************************
* ENCRYPTION FUNCTION
******************************************************************************/
void delta_txt_crypt(char *char_array, char *arg2) {
	char *pass = arg2, *s = char_array;
	int key_size = (1 + ((strlen(s) - 1)/strlen(pass))), ch_count = 0, idx = 0;	
	srand((char)pass[0]);
	while(*s != '\0') {
		if(ch_count == key_size) {
			ch_count = 0;
			idx++;
			srand((char)pass[idx]);
		}
		*s ^= (rand()%11+5);
		ch_count++;
		s++;
	}
}
/******************************************************************************
* CHAR HANDLER FUNCTIONS
******************************************************************************/
void rm_nxt_ch(char *q, int ss_ref_idx) {
	sprintf(q, "%c%s", ss_refs[ss_ref_idx], q + 1); /* replace 1st char with symbol */
	memmove(q + 1, q + 2, strlen(q + 1)); /* remove 2nd char */
}
void modify_s(char *s, int sp_flag) {
	char *p = s, space = ' ', under_s = '_';
	if(sp_flag == 0) { /* lowercase */
		while(*p != '\0') {
			*p = tolower(*p);
			p++;
		}
	} else if(sp_flag == 1) { /* lowercase + underscores for spaces (encrypt) */
		while(*p != '\0') {
			*p = tolower(*p);
			if(*p == space) *p = under_s;
			p++;
		}
	} else { /* spaces for underscores (decrypt) */
		while(*p != '\0') {
			if(*p == under_s) *p = space;
			p++;
		}
	}
}
/******************************************************************************
* DE/COMPRESSION FUNCTIONS
******************************************************************************/
void s_decompress(char ss[][151], char *s) {
	char *p = s;
	char t[151];
	while(*p != '\0') {
		if(is_ss_ref(0, *p) == 1) {
			sprintf(t, "%s%s", ss[ss_ch_index(*p)], p+1);
			sprintf(p, "%s", t);
			p = s;
		} else {
			p++;
		}
	}
}
int s_compress(char ss[][151], char *s) { /* returns # of substrings */
	modify_s(s, 1); /* lowify & ' ' => '_' */
	int ss_idx = 0, found;
	char *p = s, *r;
	while(*(p + 1) != '\0') { /* first ss instance */
		r = p + 1;
		found = 0;
		while(*(r + 1) != '\0') { /* second ss instance */
			if(*r == *p && *(r + 1) == *(p + 1)) {
				found++;
				rm_nxt_ch(r, ss_idx);
			}
			r++;
		}
		if(found > 0) { /* if ss found, restart search for compounded ss */
			store_ss(ss, p, ss_idx);
			rm_nxt_ch(p, ss_idx);
			ss_idx++;
			p = s;
		} else {
			p++;
		}
	}
	return clean_ss(ss, s, ss_idx);
}
int process_split_s(char ss[][151], char *s, char *s_compress_storage, char *arg2) {
	if(zip_info == 1) printf("\nORIGINALLY => LEN: %lu, STR: %s\n", strlen(s), s);
	int ss_total = s_compress(ss, s);
	if(zip_info == 1) printf("COMPRESSED => LEN: %lu, STR: %s\n", strlen(s), s);
	int compressed_bytes = strlen(s); /* compressed string length */
	write_bin_ss_keys(ss, ss_total, arg2); /* store ss keys in bin "password" file */
	delta_txt_crypt(s, arg2); /* encrypt text */
	strcpy(s_compress_storage, s); /* store compressed string */
	if(zip_info == 1) printf("ENCRYPTION => LEN: %lu, STR: %s\n", strlen(s), s);
	if(zip_info == 1) print_ss(ss, ss_total);
	return compressed_bytes;
}
/******************************************************************************
* (S)UB(S)TRING REFERENCE KEY ANALYSIS FUNCTIONS
******************************************************************************/
int is_ss_ref(int start, char ch) {
	char *p = &ss_refs[start];
	while(*p != '\0') {
		if(*p == ch) return 1;
		p++;
	}
	return 0;
}
int ss_ch_index(char ch_c_idx) {
	if(CH_IS_GCAP(ch_c_idx) == 1) {
		return (int)(ch_c_idx - 65);
	} else if(CH_IS_GNUM(ch_c_idx) == 1 || (ch_c_idx >= '<' && ch_c_idx <= '>')) {
		return (int)(ch_c_idx - 22);
	} else if(ch_c_idx == '#' || ch_c_idx == '$') {
		return (int)(ch_c_idx + 1);
	} else if(ch_c_idx == '@') {
		return (int)(ch_c_idx - 23);
	} else if(ch_c_idx == '[') {
		return (int)(ch_c_idx - 49);
	} else if(ch_c_idx == ']' || ch_c_idx == '^') {
		return (int)(ch_c_idx - 50);
	} else {
		return (int)(ch_c_idx - 78);
	}

}
char adjust_ref_char(char ss_ref_ch) {
	if(ss_ref_ch == '0') {
		return (char)(ss_ref_ch + 42); /* become Z */
	} else if(ss_ref_ch == '#') { /* become 9 */
		return (char)(ss_ref_ch + 22);
	} else if(ss_ref_ch == '<') { /* become $ */
		return (char)(ss_ref_ch - 24);
	} else if(ss_ref_ch == '@' || ss_ref_ch == ']') { /* become > OR [ */
		return (char)(ss_ref_ch - 2);
	} else if(ss_ref_ch == '[') { /* become @ */
		return (char)(ss_ref_ch - 27);
	} else if(ss_ref_ch == '{') { /* become ^ */
		return (char)(ss_ref_ch - 29);
	} else { /* become previous ss_refs elem */
		return (char)(ss_ref_ch - 1);
	}
}
/******************************************************************************
* (S)UB(S)TRING FUNCTIONS
******************************************************************************/
void store_ss(char ss[][151], char *p, int ss_idx) {
	sprintf(ss[ss_idx], "%c%c", *p, *(p + 1));
}
int clean_ss(char ss[][151], char *s, int ss_total) {
	char *p;
	int nest_idx = 0, nested_ss[50]; /* nested ss only ref'd by other ss, arr holds their idxs */
	for(int i = 0; i < ss_total; i++) {
		p = s;
		while(*p != '\0') {
			if(*p == ss_refs[i]) break; /* if s char == ss ref */
			p++;
		}
		if(*p == '\0') { /* if ss_refs[i] !in s, i == nested ss ref idx */
			nested_ss[nest_idx] = i; /* nested ss idxs stored sequentially relative to ss_refs order */
			nest_idx++;
		}
	}
	splice_ss(ss, nested_ss, nest_idx, ss_total); /* put nested ss vals in ss referencing them */
	trim_ss(ss, s, nested_ss, nest_idx, ss_total); /* adjust ss & s references as per rmvd nested ss */
	return (ss_total - nest_idx); /* actual ss total = original ss total - nested ss total */
}
void splice_ss(char ss[][151], int nested_ss[50], int nest_total, int ss_total) {
	for(int i = 0; i < nest_total; i++) { /* traverse nested ss array */
		for(int j = 0; j < ss_total; j++) { /* traverse ss array */
			int curr_ss_len = strlen(ss[j]);
			for(int l = 0; l < curr_ss_len; l++) { /* traverse ss */
				if(ss[j][l] == ss_refs[nested_ss[i]]) { /* if ss has nested ss reference */
					char nested_elem[151]; /* clone to hold nested ref value */
					strcpy(nested_elem, ss[nested_ss[i]]); /* clone nested ref val */
					/* splice post-nested ref ss tail to end of nested ref val */
					strcpy(&nested_elem[strlen(nested_elem)], &ss[j][l + 1]); 
					strcpy(&ss[j][l], nested_elem); /* splice nested ref val + ss end to ss ref position */
				}
			}
		}
	}
}
void trim_ss(char ss[][151], char *s, int nested_ss[50], int nest_total, int ss_total) {
	int i, j;
	for(i = 0; i < nest_total; i++) { /* delete nested-ss */
		char *p = s;
		for(j = nested_ss[i]; j < (ss_total - 1); j++) {
			strcpy(ss[j], ss[j + 1]); /* shift up non-nested ss to delete nested-ss */
			int curr_ss_len = strlen(ss[j]);
			for(int l = 0; l < curr_ss_len; l++) {
				if(is_ss_ref((nested_ss[i]+ 1), ss[j][l]) == 1)/* ss-ss references -= 1 ss_refs val as per del  */
					ss[j][l] = adjust_ref_char(ss[j][l]);
			}
		}
		for (int k = i; k < nest_total; k++) {
			nested_ss[k] = adjust_ref_char(nested_ss[k]); /* nested-ss idxs -= 1 ss_refs val as per del */
		}
		ss[j][0] = '\0';
		while(*p != '\0') { /* s ss references -= 1 ss_refs val as per del */
			if(is_ss_ref((nested_ss[i]+ 1), *p) == 1) /* if *p = nested ref */
				*p = adjust_ref_char(*p);
			p++;
		}
	}
}
void print_ss(char ss[][151], int ss_total) {
	printf("COMPR SUBSTRS:\n");
	for(int i = 0; i < ss_total; i++) {
		printf("=> %c: %s\n", ss_refs[i], ss[i]);
	}
}
