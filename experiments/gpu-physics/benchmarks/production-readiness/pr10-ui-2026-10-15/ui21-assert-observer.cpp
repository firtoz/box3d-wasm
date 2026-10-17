#include <cstdio>
#include <cstdlib>
extern "C" [[noreturn]] void __real___assert_fail(const char*,const char*,unsigned,const char*);
static void quote(FILE*f,const char*s){std::fputc('"',f);for(int i=0;s&&s[i]&&i<500;i++){unsigned char c=s[i];if(c=='"'||c=='\\')std::fputc('\\',f);std::fputc(c<32?' ':c,f);}std::fputc('"',f);}
extern "C" [[noreturn]] void __wrap___assert_fail(const char* expression,const char* file,unsigned line,const char* function){if(const char*path=std::getenv("GPU_PR10_API_TRACE"))if(FILE*f=std::fopen(path,"a")){std::fputs("{\"api\":\"ui21_native_assertion\",\"expression\":",f);quote(f,expression);std::fputs(",\"file\":",f);quote(f,file);std::fprintf(f,",\"line\":%u,\"function\":",line);quote(f,function);std::fputs("}\n",f);std::fclose(f);}__real___assert_fail(expression,file,line,function);}
