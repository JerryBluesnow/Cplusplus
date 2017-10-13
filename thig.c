#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define IBCF_SIP_THIG_BUF_LEN       1024
#define THIG_DIGITS_LENGTH          256 
#define THIG_DIGITS_LENGTH_SECOND   64
char ibcf_sip_thig_chars[THIG_DIGITS_LENGTH_SECOND] = {
        'M','i','F','b', '8','1','X','t',
        'G','c','.','3', 'U','q','N','j',
        'C','9','2','x', 'Q','m','J','f',
        'O','k','H','d', '4','y','V','r',
        'K','g','D','-', 'Y','u','R','n',
        'W','s','P','l', 'E','a','6','z',
        'S','o','L','h', 'A','5','Z','v',
        '0','w','T','p', 'I','e','B','7'
};

char ibcf_sip_thig_digits[IBCF_SIP_THIG_BUF_LEN];

/* init function must be called before encipher the input string */
inline void ibcf_sip_thig_init()
{
        int index;

        for(index=IBCF_SIP_THIG_BUF_LEN; index-- > 0; ) 
        { 
            ibcf_sip_thig_digits[index] = 0; 
        }

        for(index=THIG_DIGITS_LENGTH_SECOND; index-- > 0; ) 
        {
                ibcf_sip_thig_digits[static_cast<int>(ibcf_sip_thig_chars[index])] = index;
        }

        return;
}

/* the function is used to encipher the input string */
inline int ibcf_sip_thig_encipher(
                const char *inpStr,
                int inpLen, 
                char *res)
{
        int     len = 0;
        int     chnb = 0;
        char    key;
        char    nibble = 0;

        /* use the input length of the string to create a key for the enciphter */
        key = inpLen & 0xfc;

        /* Add the line length */
        *(res++) = ibcf_sip_thig_chars[((inpLen & 0xfc0) >> 6) & 0x3f];
        *(res++) = ibcf_sip_thig_chars[inpLen & 0x3f];

        len = 2;
        chnb = 0;

        while(inpLen > 0) {
                /*
                 *  3 consecutive bytes are encoded into 4 chars. It means that
                 *  there are trailing chars that are not of interest and that
                 *  we shall have to drop while deciphering.
                 */
                nibble = ((*inpStr) & 0xfc) >> 2;
                *(res++) = ibcf_sip_thig_chars[(nibble+(key++))& 0x3f];

                nibble = ((*(inpStr++)) & 0x03) << 4;
                nibble |= ((*inpStr) & 0xf0) >> 4;
                *(res++) = ibcf_sip_thig_chars[(nibble+(key++))& 0x3f];

                nibble = ((*(inpStr++)) & 0x0f) << 2;
                nibble |= ((*inpStr) & 0xc0) >> 6;
                *(res++) = ibcf_sip_thig_chars[(nibble+(key++))& 0x3f];

                nibble = ((*(inpStr++)) & 0x3f);
                *(res++) = ibcf_sip_thig_chars[(nibble+(key++))& 0x3f];

                len += 4;
                chnb += 4;
                inpLen -= 3;

                if(chnb == 40)
                {
                        *(res++) = 'l';
                        *(res++) = '.';
                        *(res++) = 'c';
                        len += 3;
                        chnb = 0;
                }
        }

        return len;
}

/* the function is used to decipher the input enciphered string */
inline int ibcf_sip_thig_decipher(
                const char *inpStr,
                char *resStr,
                int *resLen)
{
        int             i=0;
        int             len=0, chnb=0;
        char            *chr = NULL;
        char            key = 0;
        char            nibbles[4];
        const char      *inpCpy = inpStr;

        /* Get the key and line length */
        len = 0;
        for(i = 2; i-- > 0; ) {
                len <<= 6;
                len += (ibcf_sip_thig_digits[(int) *(inpStr++)]) & 0x3f;
        }
        len &= 0xfff;
        if(len > IBCF_SIP_THIG_BUF_LEN)
        {
                return 0;
        }

        key = len & 0xfc;

        *resLen = len;
        chnb = 0;

        while(len > 0) {
                chr = nibbles;
                for(i = 4; i-- > 0; ) {
                        *(chr++) = (ibcf_sip_thig_digits[(int) *(inpStr++)] - (key++)) & 0x3f;
                        chnb ++;
                }
                chr = nibbles;
                *resStr = ((*(chr++)) & 0x3f) << 2;
                *resStr++ |= ((*chr) & 0x30) >> 4;
                if(--len > 0) {
                        *resStr = ((*(chr++)) & 0x0f) << 4;
                        *resStr++ |= ((*chr) & 0x3c) >> 2;
                        if(--len > 0) {
                                *resStr = ((*(chr++)) & 0x03) << 6;
                                *resStr++ |= ((*chr) & 0x3f);
                                --len;
                        }
                }

                if(chnb == 40)
                {
                        *(inpStr++);
                        *(inpStr++);
                        *(inpStr++);
                        chnb = 0;
                }
        }

        return (inpStr - inpCpy);
}

void TEST_P(const char* source_string)
{
               
        char dest_string[IBCF_SIP_THIG_BUF_LEN] = {0};
        char decipher_string[IBCF_SIP_THIG_BUF_LEN] = {0};
        int decipher_len = 0;
        static int i_count=0;
        i_count++;
        printf("\r\n\r\nthe |%d| test case has been run:\n", i_count);
        ibcf_sip_thig_encipher(source_string, strlen(source_string), dest_string);
        printf("the encipher string: \r\nbefore\r\n%s \r\nlength=%d \r\nafter\r\n%s\n", source_string, strlen(source_string), dest_string);

        ibcf_sip_thig_decipher(dest_string, decipher_string, &decipher_len);
        printf("the encipher string: \r\nbefore\r\n%s \r\nafter \r\n%s \r\nlength=%d\n", dest_string, decipher_string, decipher_len);
        return;
}

int main()
{

        //const char *source_string = "sips:bob.home@ih.lucent.com;tag=abcdefg";

        ibcf_sip_thig_init();
        TEST_P("sips:bob.home@ih.lucent.com;tag=abcdefg");
        TEST_P("sips:bob.home@ih.lucent.com;tag=abcdefg;tag=10010230102301203010312Jzhanngasadawqdqqeqwerqrqweqweqw@asdasrqdasdasdasdasdasdasdasdasdasdsadasdasdasdasd");
        TEST_P("sips:bob.home.abcasdasedasdasdasdasdasdasdasdasdasdasdqweqwgfggute35esr23678uydfad1231243123123adasdasd212eadssr3343132212dadsasdasdasdadasda@ih.lucent.com;tag=abcdefg");
        TEST_P("");
        TEST_P("sips:@");
        TEST_P("sip@fg");
        TEST_P("s");
        TEST_P("sips:bob.home@@@@@@@@@@@@@@@@@@@@@@@&@&@&@&@&&@&@**@**!**!*!((@*@**#@@#ih.lucent.com;tag=abcdefg");
        TEST_P("sips:bob.home.abcasdasedasdasdasdasdasdasdasdasdasdasdqwesdasd@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@qwghhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhfggute35esr23678uydfad1231243123123adasdasd212eadssr3343132212dadsasdasdasdadasda@ih.lucent.com;tag=abcdefg");

        system("pause");
        return 0;
}
