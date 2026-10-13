#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
int main(void) {
    _Alignas(64) uint32_t a[128];
    for(unsigned i=0;i<128;++i) a[i]=UINT32_C(4294967290)+i;
    uint32_t *p=a;
    for(unsigned i=1;i<128;++i) p[i] += p[i-1] ^ (p[i] >> 3);
    uint64_t hash=0;
    for(unsigned i=0;i<128;++i) hash=(hash^a[i])*UINT64_C(1099511628211);
    printf("%" PRIu64 "\n",hash);
    return 0;
}
