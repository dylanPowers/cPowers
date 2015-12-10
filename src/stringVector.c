#include "stdarg.h"
#include "string.h"
#include "math.h"
#include "stringVector.h"

#define _STRING_VECTOR_INIT_SIZE 64

/**
 * Any Vector function can be used on a String. [contents] is a primitive 
 * c string that String can be initialized to. The length of contents is 
 * determined by running strlen(). initString() is similar except memory isn't
 * allocated for a new String.
 * @param  contents Primitive C string to initialize to.
 * @return          A pointer to the initialized string.
 * @error           S_E_NOMEMS
 */
String* initString(String* str, const char* contents, SystemErr* e) {
  size_t len = contents ? strlen(contents) : 0;
  return initByteVector(str, _STRING_VECTOR_INIT_SIZE, contents, len, e);
}

String* initStringCp(String* str, const String* copyString, SystemErr* e) {
  return initByteVector(str, copyString->_arrSize, copyString->arr,
                        copyString->length, e);
}

void deinitString(String* str) {
  deinitVector(str);
}

/**
 * @error  V_E_RANGE
 */
char String_charAt(const String* str, size_t index, VectorErr* e) {
  return *(char*) Vector_at(str, index, e);
}

int String_cmp(const String* str, const String* comparedToStr) {
  return strcmp(str->arr, comparedToStr->arr);
}

/**
 * String vector version of fgets. Works exactly like fgets except it's not
 * limited to a predefined string size.
 * The Vector given as [str] better be of char* type!
 * @error  S_E_NOMEMS
 */
void String_fgets(String* str, FILE* fd, SystemErr* se) {
  // Let's be sure to start off clean to prevent bugs, especially with strlen().
  Vector_clear(str);

  fgets(str->arr, (int) str->_arrSize, fd);
  str->length = strlen(str->arr);
  if (str->length) {
    VectorErr e = V_E_CLEAR;
    while (!se && *(char*) Vector_last(str, &e) != '\n' && !feof(fd)) {
      char tmpStr[1024];
      fgets(tmpStr, 1024, fd);
      size_t len = strlen(tmpStr);
      Vector_catPrimitive(str, tmpStr, len, se);
    }
  }
}

void String_catnprintf(String* str, size_t n, SystemErr* se, const char* fmt, ...) {
  _Vector_resize(str, n, se);

  if (!*se) {
    va_list vl;
    va_start(vl, fmt);
    vsnprintf(_Vector_calcDanglingPtr(str), n, fmt, vl);
    va_end(vl);
    str->length = strlen(str->arr);
  }
}

void String_nprintf(String* str, size_t n, SystemErr* se, const char* fmt, ...) {
  if (n > str->_arrSize) {
    _Vector_resize(str, n - str->_arrSize, se);
  }

  if (!*se) {
    va_list vl;
    va_start(vl, fmt);
    vsnprintf(str->arr, n, fmt, vl);
    va_end(vl);
    str->length = strlen(str->arr);
  }
}

int String_toi(const String* str, int base) {
  int num = 0;
  for (int i = 0; i < str->length; ++i) {
    VectorErr ve = V_E_CLEAR;
    char* c = Vector_at(str, str->length - 1 - i, &ve);
    if (*c < '0' + base && *c > '0') {
      num += (*c - '0') * pow(base, i);
    } else if (*c < base - 10 + 'A' && *c > 'A') {
      num += (*c - 'A' + 10) * pow(base, i);
    } else {
      return 0;
    }
  }

  return num;
}

/**
 * @error  S_E_NOMEMS
 */
void String_tok(const String* str, Vector* tokenContainer,
                const char* delimiters, SystemErr* e) {
  Vector_clear(tokenContainer);

  char* tokenized = (char*) malloc(str->length + 1);
  memcpy(tokenized, str->arr, str->length + 1);
  char* token = strtok(tokenized, delimiters);
  while (token != NULL) {
    String strToken = {};
    initString(&strToken, token, e);
    Vector_add(tokenContainer, &strToken, e);

    deinitString(&strToken);
    token = strtok(NULL, delimiters);
  }

  free(tokenized);
}
