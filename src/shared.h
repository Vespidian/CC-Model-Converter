#ifndef SHARED_H_
#define SHARED_H_

typedef union{
	struct{char c1, c2, c3, c4;};
	struct{short s1, s2;};
	int i;
	float f;
}TypeConverter_tu;

typedef struct{
	union{
		struct{float x, y, z;};
		float v[3];
	};
}Vector3;
typedef struct{
	union{
		struct{float x, y;};
		float v[2];
	};
}Vector2;

typedef struct{
	Vector3 pos;
	Vector2 uv;
	TypeConverter_tu diffuse;
}Vertex;

typedef unsigned short Triangle[3];

// Used to convert windows filepaths which use backslashes
void ReplaceChars(char *str, char find, char replace){
	unsigned int ptr = 0;
	while(str[ptr] != 0){
		if(str[ptr] == find){
			str[ptr] = replace;
		}
		ptr++;
	}
}

unsigned int CountChars(char *str, char find, unsigned int length){
	unsigned int counter = 0;
	for(int i = 0; i < length; i++){
		if(str[i] == find){
			counter++;
		}
	}
	return counter;
}

// bool strnstr(const char *s1, const char *s2, size_t n) {
//     // simplistic algorithm with O(n2) worst case
//     size_t i, len;
//     char c = *s2;

//     if (c == '\0')
//         return false;

//     for (len = strlen(s2); len <= n; n--, s1++) {
//         if (*s1 == c) {
//             for (i = 1;; i++) {
//                 if (i == len)
//                     return true;
//                 if (s1[i] != s2[i])
//                     break;
//             }
//         }
//     }
//     return false;
// }
bool strnstr(char *haystack, char *needle, unsigned int length){
	unsigned int needle_len = strlen(needle);
	if(length < needle_len || needle_len == 0){
		return false;
	}
	for(int i = 0; i < length; i++){
		if(strncmp(haystack + i, needle, needle_len) == 0){
			return true;
		}
	}
	return false;
}

#endif