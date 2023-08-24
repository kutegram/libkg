#include <QtGlobal>

#ifdef Q_OS_SYMBIAN

extern unsigned int __aeabi_uidivmod(unsigned numerator, unsigned denominator);

int __aeabi_idiv(int numerator, int denominator)
{
    int neg_result = (numerator ^ denominator) & 0x80000000;
    int result = __aeabi_uidivmod ((numerator < 0) ? -numerator : numerator, (denominator < 0) ? -denominator : denominator);
    return neg_result ? -result : result;
}

unsigned __aeabi_uidiv(unsigned numerator, unsigned denominator)
{
    return __aeabi_uidivmod (numerator, denominator);
}

//extern int __aeabi_uidivmod(unsigned int a, unsigned int b);
//extern int __aeabi_idivmod(int a, int b);
//int __aeabi_idiv(int a, int b)
//{
//	return __aeabi_idivmod(a, b);
//}

//int __aeabi_uidiv(unsigned int a, unsigned int b)
//{
//	return __aeabi_uidivmod(a, b);
//}

#endif
