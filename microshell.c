#include <unistd.h>
#include <stdlib.h>
#include <string.h>

size_t	ft_strlen(char *s)
{
	size_t	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

void	_puterr(char *err, char *path)
{
	write(2, err, ft_strlen(err));
	if (path)
	{
		write(2, path, ft_strlen(path));
		write(2, "\n", 1);
	}
	exit(1);
}

char	**_subargv(char *argv[], int start, int end)
{
	char	**res;
	int		i;

	res = malloc(sizeof(char *) * (end - start + 1));
	if (res == NULL)
		return (NULL);
	i = 0;
	while (start < end)
		res[i++] = argv[start++];
	res[i] = NULL;
	return (res);
}

int	main(int argc, char *argv[], char *envp[])
{
	int		i, pos_semicolon, start, end;
	int		fd[2], fd_in;
	pid_t	pid;
	char	**av;

	i = 1;
	while (i < argc)
	{
		pos_semicolon = start = end = i;
		while (pos_semicolon < argc && strcmp(argv[pos_semicolon], ";"))
			pos_semicolon++;
		fd_in = 0;
		while (start < pos_semicolon)
		{
			end = start;
			while (end < pos_semicolon && strcmp(argv[end], "|"))
				end++;
			av = _subargv(argv, start, end);
			if (av == NULL)
				_puterr("error: fatal\n", NULL);
			if (pipe(fd) == -1)
				_puterr("error: fatal\n", NULL);
			pid = fork();
			if (pid == -1)
				_puterr("error: fatal\n", NULL);
			if (pid == 0)
			{
				if (dup2(fd_in, 0) == -1)
					_puterr("error: fatal\n", NULL);
				if (end < pos_semicolon && dup2(fd[1], 1) == -1)
					_puterr("error: fatal\n", NULL);
				close(fd_in);
				close(fd[0]);
				close(fd[1]);
				if (strcmp(av[0], "cd") == 0)
				{
					if (end - start != 2)
						_puterr("error: cd: bad arguments\n", NULL);
					if (chdir(av[1]))
						_puterr("error: cd: cannot change directory to ", av[1]);
				}
				else if (execve(av[0], av, envp))
					_puterr("error: cannot execute ", av[0]);
				free(av);
				exit(0);
			}
			else
			{
				waitpid(pid, NULL, 0);
				close(fd[1]);
				if (fd_in)
					close(fd_in);
				fd_in = fd[0];
				free(av);
			}
			start = end + 1;
		}
		close(fd_in);
		i = pos_semicolon + 1;
	}
	return (0);
}
