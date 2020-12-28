typedef __int128 int128_t

int128_t MirrorHorizontalDiscs(int128_t b)
{
    b    = ((b >> 1) & 0x5555555555555555) | ((b << 1) & 0xAAAAAAAAAAAAAAAA);
    b    = ((b >> 2) & 0x3333333333333333) | ((b << 2) & 0xCCCCCCCCCCCCCCCC);
    return ((b >> 4) & 0x0F0F0F0F0F0F0F0F) | ((b << 4) & 0xF0F0F0F0F0F0F0F0);
}