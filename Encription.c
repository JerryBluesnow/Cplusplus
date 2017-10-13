#include <stdio.h>

#define sfw_dfa_thig_bufLen 1024

char sfw_dfa_thig_chars[64] = {
        '4','y','V','r',  'O','k','H','d',
        'Y','u','R','n',  'K','g','D','-',
        'U','q','N','j',  'G','c','.','3',
        'Q','m','J','f',  'C','9','2','x',
        'M','i','F','b',  '8','1','X','t',
        'I','e','B','7',  '0','w','T','p',
        'E','a','6','z',  'W','s','P','l',
        'A','5','Z','v',  'S','o','L','h'
};

char sfw_dfa_thig_digits[256];

inline int sfw_dfa_thig_encipher(
                char      *inpStr,
                int       inpLen,
                char      *res)
{
        int     len, chnb;
        char    key, nibble;
        int     index = 0;
        int     source_index = 0;

        /* a log is for the entry of the function */
        printf(" the Entry of  function: sfw_dfa_thig_encipher\n");

        /* use the input length of the string to create a key for the enciphter */
        key = inpLen & 0xfc;

        /* Add the line length */
        res[index++] = sfw_dfa_thig_chars[((inpLen & 0xfc0) >> 6) & 0x3f];
        res[index++] = sfw_dfa_thig_chars[inpLen & 0x3f];

        len = 2;
        chnb = 0;


        while(inpLen > 0) {
                /* 3 consecutive bytes are encoded into 4 chars. It means that
                 *            there are trailing chars that are not of interest and
                 *                       that we shall have to drop while deciphering.
                 *                                */

                nibble = ((inpStr[source_index]) & 0xfc) >> 2;
                res[index++] = sfw_dfa_thig_chars[(nibble+(key++))& 0x3f];

                nibble = ((inpStr[source_index++]) & 0x03) << 4;
                nibble |= ((inpStr[source_index]) & 0xf0) >> 4;
                res[index++] = sfw_dfa_thig_chars[(nibble+(key++))& 0x3f];

                nibble = ((inpStr[source_index++]) & 0x0f) << 2;
                nibble |= ((inpStr[source_index]) & 0xc0) >> 6;
                res[index++] = sfw_dfa_thig_chars[(nibble+(key++))& 0x3f];

                nibble = ((inpStr[source_index++]) & 0x3f);
                res[index++] = sfw_dfa_thig_chars[(nibble+(key++))& 0x3f];

                len += 4;
                chnb += 4;
                inpLen -= 3;

                if(chnb == 40)
                {
                        res[index++] = 'l';
                        res[index++] = '.';
                        res[index++] = 'c';
                        len += 3;
                        chnb = 0;
                }
        }

        return len;
}

inline int sfw_dfa_thig_decipher(
                char    *inpStr,
                char    *resStr,
                int     *resLen)
{
        int    i=0;
        int    len=0, chnb=0;
        char   *chr = NULL;
        char   key, nibbles[4];
        char   *inpCpy = NULL;
        int     source_index = 0;

        printf("Entering sfw_dfa_thig_decipher\n");

        key = 0;
        inpCpy = inpStr;

        /* Get the key and line length */
        len = 0;
        for(i = 2; i-- > 0; ) {
                len <<= 6;
                len += (sfw_dfa_thig_digits[(int) inpStr[source_index++]]) & 0x3f;
        }

        len &= 0xfff;

        if(len > sfw_dfa_thig_bufLen)
        {
                printf("length(%d) > sfw_dfa_thig_bufLen(%d)\n", len, sfw_dfa_thig_bufLen);
                return 0;
        }

        key = len & 0xfc;

        printf("length(%d)\n", len);
        *resLen = len;
        chnb = 0;

        while(len > 0) {
                int index = 0;
                chr = nibbles;
                for(i = 4; i-- > 0; ) {
                        chr[index++] = (sfw_dfa_thig_digits[(int) inpStr[source_index++]] - (key++)) & 0x3f;
                        chnb++;
                }

                index = 0;
                resStr[source_index] = ((chr[index++]) & 0x3f) << 2;
                resStr[source_index++] |= ((chr[index]) & 0x30) >> 4;
                if(--len > 0) {
                        resStr[source_index] = ((chr[index++]) & 0x0f) << 4;
                        resStr[source_index++] |= ((chr[index]) & 0x3c) >> 2;
                        if(--len > 0) {
                                resStr[source_index] = ((chr[index++]) & 0x03) << 6;
                                resStr[source_index++] |= ((chr[index]) & 0x3f);
                                --len;
                        }
                }

                if(chnb == 40)
                {
                        inpStr[source_index++];
                        inpStr[source_index++];
                        inpStr[source_index++];
                        chnb = 0;
                }
        }

        return (inpStr - inpCpy);
}

inline void sfw_dfa_thig_init()
{
    int i;

    for(i=256; i-- > 0; ) { sfw_dfa_thig_digits[i] = 0; }
    for(i=64; i-- > 0; ) {
        sfw_dfa_thig_digits[(int) sfw_dfa_thig_chars[i]] = i;
    }
    return;
}


int main()
{
        char *source_string = "sips:bob.home@ih.lucent.com;tag=abcdefg";
        char dest_string[256] = {0};
        char decipher_string[256] = {0};
        int decipher_len = 0;

        sfw_dfa_thig_init();
        sfw_dfa_thig_encipher(source_string, strlen(source_string), dest_string);
        printf("the encipher string, \r\nbefore\r\n%s, \r\nlength=%d,\r\nafter\r\n%s\n", source_string, strlen(source_string), dest_string);


        sfw_dfa_thig_decipher(dest_string, decipher_string, &decipher_len);
        printf("the encipher string, \r\nbefore\r\n%s, \r\nafter\r\n%s, \r\nlength=%d\n", dest_string, decipher_string, decipher_len);

        return 0;
}