#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>

#define C_RESET "\033[0m"
#define C_RED "\033[31m"
#define C_GREEN "\033[32m"
#define C_YELLOW "\033[33m"
#define C_BLUE "\033[34m"
#define C_PURPLE "\033[35m"
#define C_CYAN "\033[36m"

void ClearFile(const char* filename);
off_t ParseFile(const char* filename, char** text, char*** indexes, int* index_num);
off_t MeasureFile(const char* filename);
int CountNewLines(char* text, const off_t file_length);
void FillIndexes(char** text, char*** indexes, off_t file_length);

void PickCompareType(const char compare_type, int (**how_to_compare)(char*, char*));

void MergeSort(char** s, const int s_length, int (*how_to_compare)(char*, char*));
void Sort2Arrays(char** s, const int s_length, int how_to_compare(char*, char*));
void Sort2El(int* i, int* j, const int s_length, char** s, char** sorted_s, int (*how_to_compare)(char *a, char *b));

void PrintText(char* text, const off_t text_length, const char* sorted_filename);
void PrintTextByIndex(char** indexes, const int index_num, const char* sorted_filename);
void PrintArray(char* s, int const s_length);
void ArrayCopy(char** arr_copy, char** arr_origin, const int s_length);

int CompareCharUp(char* a, char* b);
int CompareCharDown(char* a, char* b);
int CompareLastCharUp(char*a, char* b);

//const size_t MAX_LENGTH = 1024;
const int EDA = 3802;
const char* TEXT_SEPARATOR = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";

enum compare_results {
    FIRST_HIGHER = -1,
    EQUAL = 0,
    SECOND_HIGHER = 1
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf(C_RED "Enter file name next time -_-\n" C_RESET);
        exit(EXIT_FAILURE);
    }

    const char* filename = argv[1];
    printf(C_CYAN "DEBUG: filename = %s\n", filename);
    const char* sorted_filename = argv[2];
    printf(C_CYAN "DEBUG: sorted_filename = %s\n", sorted_filename);

    ClearFile(sorted_filename); // норм или стрем???????

    char* text = {};
    char** indexes = {};
    int index_num = 0;
    off_t file_length = ParseFile(filename, &text, &indexes, &index_num);
    printf(C_CYAN "DEBUG: File was read, file_length = %lld, index_num = %d\n" C_RESET, file_length, index_num);
    
    char compare_type = '\0';
    int (*how_to_compare)(char*, char*) = NULL;
    
    compare_type = 'u';
    PickCompareType(compare_type, &how_to_compare);

    //sorting by going up
    //int (*how_to_compare)(char*, char*) = &CompareCharUp;
    printf(C_YELLOW "DEBUG: Started sorting\n" C_RESET);
    MergeSort(indexes, index_num, how_to_compare);
    printf(C_YELLOW "DEBUG: Ended sorting\n" C_RESET);

    PrintTextByIndex(indexes, index_num, sorted_filename); 
    printf(C_CYAN "DEBUG: index_num = %d\n" C_RESET, index_num);


    compare_type = 'l';
    PickCompareType(compare_type, &how_to_compare);
    printf(C_YELLOW "DEBUG: Started sorting\n" C_RESET);
    MergeSort(indexes, index_num, how_to_compare);
    printf(C_YELLOW "DEBUG: Ended sorting\n" C_RESET);


    PrintTextByIndex(indexes, index_num, sorted_filename); 
    printf(C_CYAN "DEBUG: index_num = %d\n" C_RESET, index_num);

    
    PrintText(text, file_length, sorted_filename); // prints original

    free(text);
    free(indexes);
}


void ClearFile(const char* filename) {
    printf(C_PURPLE "DEBUG: Started ClearFile\n" C_RESET);
    assert(filename);

    FILE* fp = fopen(filename, "w");
    fclose(fp);

    printf(C_PURPLE "DEBUG: Ended ClearFile\n" C_RESET);
}


off_t ParseFile(const char* filename, char** text, char*** indexes, int* index_num) {
    printf(C_PURPLE "DEBUG: Started ParseFile\n" C_RESET);
    assert(text);
    assert(indexes);
    assert(filename);
    assert(index_num);

    off_t file_length = MeasureFile(filename);
    printf(C_CYAN "DEBUG: file_length = %lld\n" C_RESET, file_length);

    *text = (char*) calloc((size_t) file_length, sizeof(char));

    FILE* fp = fopen(filename, "r");
    assert(fp);
    fread(*text, sizeof(char), (size_t) file_length, fp); // оптимально ли читать весь файл сразу?
    fclose(fp);

    //PrintText(*text, file_length);

    *index_num = CountNewLines(*text, file_length);
    printf(C_CYAN "DEBUG: index_num = %d\n" C_RESET, *index_num);

    (*indexes) = (char**) calloc((size_t) *index_num + 2, sizeof(char*));
    (*indexes)[0] = *text; // нулевой индекс
    (*indexes)[*index_num] = *text + file_length; // последний символ
    (*indexes)[*index_num + 1] = (char*) EDA; // опа у меня тоже есть конарейки

    FillIndexes(text, indexes, file_length);
    assert((*indexes)[*index_num + 1] == (char*) EDA);
    
    printf(C_PURPLE "DEBUG: Ended ParseFile\n" C_RESET);
    return file_length;
}

off_t MeasureFile(const char* filename) {
    printf(C_PURPLE "DEBUG: Started MeasureFile\n" C_RESET);

    struct stat file_param = {};
    stat(filename, &file_param);
    printf(C_CYAN "DEBUG: file size = %lld\n" C_RESET, file_param.st_size);

    printf(C_PURPLE "DEBUG: Ended MeasureFile\n" C_RESET);
    return file_param.st_size;
}

int CountNewLines(char* text, const off_t file_length) {
    printf(C_PURPLE "DEBUG: Started CountNewLines\n" C_RESET);
    assert(text);

    int lines_num = 0;
    for (int i = 0; i < file_length; i++) {
        assert(0 <= i); assert(i < file_length);
        
        //printf(C_CYAN "DEBUG: i = %d\n" C_RESET, i);
        if (text[i] == '\n') {
            lines_num++;
        }
        //else
        //    printf(C_CYAN "DEBUG: symbol = %c\n" C_RESET, text[i]);
    }

    printf(C_CYAN "DEBUG: lines_num = %d\n" C_RESET, lines_num);
    printf(C_PURPLE "DEBUG: Ended CountNewLines\n" C_RESET);
    return lines_num;
}

void FillIndexes(char** text, char*** indexes, const off_t file_length) {
    printf(C_PURPLE "DEBUG: Started FillIndexes\n" C_RESET);
    assert(text); assert(indexes);
    printf(C_CYAN "DEBUG: file_length = %lld\n" C_RESET, file_length);

    int index_ind = 1;
    for (int i = 0; i < file_length; i++) {
        assert(0 <= i); assert(i < file_length);
        if ((*text)[i] == '\n') {
            (*indexes)[index_ind] = (*text) + i + 1;
            index_ind++;
        }
    }
    
    printf(C_CYAN "DEBUG: last index_ind = %d\n" C_RESET, index_ind);
    printf(C_PURPLE "DEBUG: Ended FillIndexes\n" C_RESET);
}


void PickCompareType(const char compare_type, int (* *how_to_compare)(char*, char*)) {
    printf(C_PURPLE "DEBUG: Started PickType\n" C_RESET);
    assert(compare_type);
    printf(C_CYAN "DEBUG: compare_type = %c\n" C_RESET, compare_type);

    switch (compare_type) {
        case 'u':
            *how_to_compare = CompareCharUp;
            break;
        case 'd':
            *how_to_compare = CompareCharDown;
            break;
        case 'l':
            *how_to_compare = CompareLastCharUp;
            break;
        default:
            printf("Unknown type\n");
            assert(0);
    }
    printf(C_PURPLE "DEBUG: Ended PickType\n" C_RESET);
}


void MergeSort(char** s, const int s_length, int how_to_compare(char*, char*)) {
    //printf(C_PURPLE "DEBUG: Started MergeSort\n" C_RESET);
    assert(s);
    assert(s_length > 0);

    if (s_length > 1) {
        MergeSort(s, s_length / 2, how_to_compare);
        MergeSort(s + s_length / 2, s_length - s_length / 2, how_to_compare);

        Sort2Arrays(s, s_length, how_to_compare);

        //PrintArray(s, s_length);
    }
    //printf(C_PURPLE "DEBUG: Ended MergeSort\n" C_RESET);
}

void Sort2Arrays(char** s, const int s_length, int how_to_compare(char*, char*)) {
    char** temp = (char**) calloc((size_t) s_length, sizeof(s[0]));

    int i = 0;
    int j = 0;

    while (1) {
        Sort2El(&i, &j, s_length, s, temp, how_to_compare);

        if (i == s_length / 2) {
            for (; j < s_length - s_length / 2; j++)
                temp[i + j] = s[i + j];
            break;
        }

        else if (j == s_length - s_length / 2) {
            for (; i < s_length / 2; i++)
                temp[i + j] = s[i];
            break;
        }
    }
    assert(i <= s_length / 2);
    assert(j <= s_length - s_length / 2);

    ArrayCopy(s, temp, s_length);
    free(temp);
}

// picks right element and moves cursors i and j
void Sort2El(int* i, int* j, const int s_length, char** s, char** sorted_s, int (*how_to_compare)(char* a, char* b)) {
    assert(i); assert(j);
    assert(s_length > 0);
    assert(s); assert(sorted_s);

    if ((*how_to_compare)(s[*i], s[s_length / 2 + *j]) == FIRST_HIGHER) {
        sorted_s[*i + *j] = s[*i];
        *i += 1;
    }

    else {
        sorted_s[*i + *j] = s[s_length / 2 + *j];
        *j += 1;
    }
}



void PrintText(char* text, const off_t text_length, const char* sorted_filename) {
    printf(C_PURPLE "DEBUG: Started PrintText\n" C_RESET);
    assert(text);
    assert(text_length > 0);
    assert(sorted_filename);
    printf(C_CYAN "DEBUG: text_length = %lld\n" C_RESET, text_length);
    printf(C_CYAN "DEBUG: sorted_filename = %s\n" C_RESET, sorted_filename);

    FILE* fp = fopen(sorted_filename, "a");

    for (int i = 0; i < text_length; i++) {
        fprintf(fp, "%c", text[i]);
    }

    fprintf(fp, "%s", TEXT_SEPARATOR);
    fclose(fp);
    printf(C_PURPLE "DEBUG: Ended PrintText\n" C_RESET);
}

void PrintTextByIndex(char** indexes, const int index_num, const char* sorted_filename) {
    printf(C_PURPLE "DEBUG: Started PrintTextByIndex\n" C_RESET);
    assert(indexes);
    assert(index_num > 0);
    assert(sorted_filename);
    printf(C_CYAN "DEBUG: index_num = %d\n" C_RESET, index_num);
    printf(C_CYAN "DEBUG: sorted_filename = %s\n" C_RESET, sorted_filename);
    printf(C_CYAN "DEBUG: pointer to indexes = %p\n" C_RESET, indexes);

    FILE* fp = fopen(sorted_filename, "a");
    assert(fp);
    printf(C_CYAN "DEBUG: pointer to opened file \"%s\" = %p\n" C_RESET, sorted_filename, fp);

    for (int i = 0; i < index_num; i++) {
        //printf(C_CYAN "DEBUG: i = %d; indexes[i] = %p\n" C_RESET, i, indexes[i]);
        
        for (char* ind = indexes[i]; *ind != '\n'; ind++) {
            fprintf(fp, "%c", *ind); // переписать на putchar()???????
        }
        fprintf(fp, "\n");

        assert(indexes[i + 1] != (char*) EDA);
    }

    fprintf(fp, "%s", TEXT_SEPARATOR);
    fclose(fp);
    printf(C_PURPLE "DEBUG: Ended PrintTextByIndex\n" C_RESET);
}


void PrintArray(char** s, const int s_length) {
    printf(C_PURPLE "DEBUG: Started PrintArray\n" C_RESET);
    assert(s);
    assert(s_length >= 0);
    printf(C_CYAN "DEBUG: s_length = %d\n" C_RESET, s_length);


    for (int i = 0; i < s_length; i++)
        printf("%s ", s[i]);

    printf("\n");
    printf(C_PURPLE "DEBUG: Ended PrintArray\n" C_RESET);
}


void ArrayCopy(char** arr_copy, char** arr_origin, const int arr_length) {
    assert(arr_copy);
    assert(arr_origin);
    assert(arr_length >= 0);
    
    for (int i = 0; i < arr_length; i++)
        arr_copy[i] = arr_origin[i];
}


int CompareCharUp(char* a, char* b) {
    assert(a); assert(b);

    int i = 0, j = 0;

    if (a[0] == '\n')
        return FIRST_HIGHER;
    if (b[0] == '\n')
        return SECOND_HIGHER;

    while (a[i] != '\n' && b[j] != '\n') {
        while (!isalpha(a[i])) {
            i++;
            if (a[i] == '\n')
                return FIRST_HIGHER;
        }
        while (!isalpha(b[j])) {
            j++;
            if (b[j] == '\n')
                return SECOND_HIGHER;
        }

        if (tolower(a[i]) > tolower(b[i]))
            return SECOND_HIGHER;
        if (tolower(a[i]) < tolower(b[i]))
            return FIRST_HIGHER;
        
        i++;
        j++;
    }

    if (a[i] == '\n')
        return FIRST_HIGHER;
    if (b[j] == '\n')
        return SECOND_HIGHER;

    return EQUAL;
}

int CompareCharDown(char* a, char* b) {
    assert(a); assert(b);

    int i = 0, j = 0;
    if (a[0] == '\n')
        return SECOND_HIGHER;
    if (b[0] == '\n')
        return FIRST_HIGHER;

    while (a[i] != '\n' && b[i] != '\n') {
        while (!isalnum(a[i]) || isspace(a[i])) {
            i++;
            if (a[i] == '\n')
                return SECOND_HIGHER;
        }
        while (!isalnum(b[j]) || isspace(b[j])) {
            j++;
            if (b[j] == '\n')
                return FIRST_HIGHER;
        }

        if (tolower(a[i]) > tolower(b[j]))
            return FIRST_HIGHER;
        if (tolower(a[i]) < tolower(b[j]))
            return SECOND_HIGHER;
        
        i++;
        j++;
    }

    if (a[i] == '\n')
        return SECOND_HIGHER;
    if (b[j] == '\n')
        return FIRST_HIGHER;

    return EQUAL;
}

int CompareLastCharUp(char*a, char* b) {
    assert(a); assert(b);

    int i = 0, j = 0;

    if (a[0] == '\n')
        return FIRST_HIGHER;
    if (b[0] == '\n')
        return SECOND_HIGHER;

    // перемещаемся в концы строк
    while (a[i] != '\n')
        i++;
    while (b[j] != '\n')
        j++;
    i--;
    j--;

    while (i >= 0 && j >= 0) {
        while (!isalpha(a[i])) {
            i--;
            if (i < 0)
                return FIRST_HIGHER;
        }
        while (!isalpha(b[j])) {
            j--;
            if (j < 0)
                return SECOND_HIGHER;
        }

        if (tolower(a[i]) > tolower(b[j]))
            return SECOND_HIGHER;
        if (tolower(a[i]) < tolower(b[j]))
            return FIRST_HIGHER;
        
        i--;
        j--;
    }
    if (i < 0)
        return FIRST_HIGHER;
    if (j < 0)
        return SECOND_HIGHER;

    return EQUAL;
}