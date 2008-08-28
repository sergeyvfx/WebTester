#include <stdio.h>
#include <string.h>

int kmp (char *str, char *substr)
{
  char s[100];
  int f[100], k, n, i, m;
  f[0]=0; f[1]=0;

  sprintf (s, "%s$%s", substr, str);

  n=strlen (s);
  m=strlen (substr);
  for (i=2; i<=n; i++)
    {
      k=f[i-1];
      while (k>0 && s[k]!=s[i-1])
        k=f[k];
      if (s[i-1]==s[k])
        k++;
      f[i]=k;
      
      if (k==m)
        return i-2*m-1;
    }
  return -1;
}

int main (void)
{
  printf ("%d\n", kmp ("ccjkbab", "bab"));
  return 0;
}
