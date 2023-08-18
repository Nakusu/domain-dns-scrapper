#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char	*strjoin(char const *s1, char const *s2)
{
	char	*stock;
	int		total;
	int		i;
	int		p;

	if (!s1 || !s2)
		return (NULL);
	p = 0;
	i = 0;
	total = (int)strlen(s1) + (int)strlen(s2) + 1;
	if (!(stock = malloc(sizeof(char) * total)))
		return (0);
	while (p < (int)strlen(s1))
		stock[i++] = s1[p++];
	p = 0;
	while (p < (int)strlen(s2))
		stock[i++] = s2[p++];
	stock[i] = 0;
	return (stock);
}