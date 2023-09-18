/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecullier <ecullier@student.42perpignan.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/04 07:35:25 by vfuster-          #+#    #+#             */
/*   Updated: 2023/09/15 15:43:03 by ecullier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

void execute_echo(char *arguments[], int argument_count)
{
    int no_newline = 0;
    int i = 1;

    if (argument_count > 1 && strcmp(arguments[1], "-n") == 0)
    {
        no_newline = 1;
        i++;
    }
    while (i < argument_count)
    {
        printf("%s", arguments[i]);
        if (i < argument_count - 1)
            printf(" ");
        i++;
    }
    if (!no_newline)
        printf("\n");
}

void execute_cd(char *path)
{
    if (path == NULL)
    {
        fprintf(stderr, "cd: missing argument\n");
    }
    else
    {
        if (chdir(path) != 0)
        {
            perror("cd");
        }
    }
}

void execute_pwd()
{
    char current_directory[1024];

    if (getcwd(current_directory, sizeof(current_directory)) != NULL)
    {
        printf("%s\n", current_directory);
    }
    else
    {
        perror("pwd");
    }
}

void execute_export(char *var_name, char *var_value)
{
    if (var_name != NULL && var_value != NULL)
    {
        if (setenv(var_name, var_value, 1) != 0)
        {
            perror("export");
        }
    }
    else
    {
        printf("export: missing arguments\n");
    }
}

void execute_unset(char *var_name)
{
    if (var_name != NULL)
    {
        if (unsetenv(var_name) != 0)
        {
            perror("unset");
        }
    }
    else
    {
        printf("unset: missing argument\n");
    }
}

void execute_env()
{
    extern char **environ;
    char *env_var;
    int i = 0;

    while (environ[i] != NULL)
    {
        env_var = environ[i];
        printf("%s\n", env_var);
        i++;
    }
}

void execute_exit(int exit_status)
{
    exit(exit_status);
}

int est_builtin(const char *command)
{
    if (strcmp(command, "echo") == 0 ||
        strcmp(command, "cd") == 0 ||
        strcmp(command, "pwd") == 0 ||
        strcmp(command, "export") == 0 ||
        strcmp(command, "unset") == 0 ||
        strcmp(command, "env") == 0 ||
        strcmp(command, "exit") == 0)
    {
        return 1;
    }
    return 0;
}

void execute_builtin(const char *command, char **arguments, int argument_count)
{
    if (strcmp(command, "echo") == 0)
        execute_echo(arguments, argument_count);
    else if (strcmp(command, "cd") == 0)
        execute_cd(arguments[1]);
    else if (strcmp(command, "pwd") == 0)
        execute_pwd();
    else if (strcmp(command, "export") == 0)
        execute_export(arguments[1], arguments[2]);
    else if (strcmp(command, "unset") == 0)
        execute_unset(arguments[1]);
    else if (strcmp(command, "env") == 0)
        execute_env();
    else if (strcmp(command, "exit") == 0)
        execute_exit(0);
}


int	is_escape_char(char c)
{
	return (c =='\\');
}

int	is_quote_char(char c)
{
	return (c == '\'' || c == '"');
}

char *process_quote(char *input)
{
    char *output;
    char *input_ptr;
    char *output_ptr;
    int escaped;
    int single_quoted;

    output = (char *)malloc(ft_strlen(input) + 1);
    if (!output)
    {
        perror("Erreur d'allocation de mémoire");
        exit(EXIT_FAILURE);
    }
    input_ptr = input;
    output_ptr = output;
    escaped = 0;
    single_quoted = 0;

    while (*input_ptr != '\0')
    {
        if (is_escape_char(*input_ptr) && !escaped && !single_quoted)
        {
            escaped = 1;
            input_ptr++;
        }
        else if (is_quote_char(*input_ptr) && !escaped)
        {
            if (*input_ptr == '\'')
            {
                single_quoted = !single_quoted;
            }
            input_ptr++;
        }
        else
        {
            *output_ptr = *input_ptr;
            input_ptr++;
            output_ptr++;
            escaped = 0;
        }
    }
    *output_ptr = '\0';
    return (output);
}

char *trouver_chemin_absolu(const char *command_name)
{
    char *chemin_absolu = NULL;
    char *path = getenv("PATH"); // Récupérez la variable d'environnement PATH

    if (path == NULL)
    {
        fprintf(stderr, "La variable d'environnement PATH n'est pas définie\n");
        return NULL;
    }
    char *path_copie = strdup(path); // Faites une copie de la variable PATH pour la diviser
    char *repertoire = strtok(path_copie, ":"); // Divisez la variable PATH en répertoires
    while (repertoire != NULL)
    {
        // Construisez le chemin absolu en combinant le répertoire et le nom de la commande
        chemin_absolu = (char *)malloc(MAX_PATH_LENGTH);
        if (chemin_absolu == NULL)
        {
            perror("Erreur lors de l'allocation de mémoire");
            exit(EXIT_FAILURE);
        }
        ft_strcpy(chemin_absolu, repertoire);
        ft_strcat(chemin_absolu, "/");
        ft_strcat(chemin_absolu, command_name); // Concaténez le nom de la commande
        // Vérifiez si le fichier existe et est exécutable
        if (access(chemin_absolu, X_OK) == 0)
        {
            free(path_copie);
            return chemin_absolu; // Le chemin absolu a été trouvé
        }
        free(chemin_absolu);
        repertoire = ft_strtok(NULL, ":"); // Passez au répertoire suivant
    }
    free(path_copie);
    fprintf(stderr, "Aucun chemin absolu trouvé pour %s\n", command_name);
    return NULL;
}

int execute_command(t_command *command)
{
    int status;
    pid_t pid = fork();

    if (pid == 0)
    { // Code du processus enfant
        char *full_path = trouver_chemin_absolu(command->command_name);
        if (full_path == NULL)
        {
            perror("Erreur lors de la recherche du chemin de l'exécutable");
            exit(EXIT_FAILURE);
        }

        // Remplacez le descripteur de fichier standard si nécessaire
        if (command->input_redirection)
        {
            int fd = open(command->input_redirection, O_RDONLY);
            if (fd == -1)
            {
                perror("Erreur lors de l'ouverture du fichier d'entrée");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        if (command->output_redirection)
        {
            int flags = O_WRONLY | O_CREAT;
            if (command->append_output)
            {
                flags |= O_APPEND;
            }
            int fd = open(command->output_redirection, flags, 0644);
            if (fd == -1)
            {
                perror("Erreur lors de l'ouverture du fichier de sortie");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        execve(full_path, command->arguments, environ);
        perror("Erreur lors de l'exécution de la commande");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    { // Code du processus parent
        wait(&status);
        command->last_exit_status = WEXITSTATUS(status);
    }
    else
    { // Gestion de l'erreur de fork
        perror("Erreur de fork");
    }
    return 0;
}


void	ctrl_c_handler(int signo)
{
	(void)signo;
	ctrl_c_pressed = 1;
}

void	ctrl_d_handler(int signo)
{
	(void)signo;
	printf("Quit\n");
	exit(EXIT_SUCCESS);
}

void	analyze_tokens(t_command *command)
{
	int	i;
	char	*env_variable;
	char	*exit_status_str;

	i = 0;
	while (i < command->argument_count)
	{
		if (ft_strcmp(command->arguments[i], ">") == 0)
		{
			if (i < command->argument_count - 1)
			{
				command->output_redirection = command->arguments[i + 1];
				command->arguments[i] = NULL;
				command->arguments[i + 1] = NULL;
				i++;
			}
		}
		else if (ft_strcmp(command->arguments[i], "<") == 0)
		{
			if (i < command->argument_count - 1)
			{
				command->input_redirection = command->arguments[i + 1];
				command->arguments[i] = NULL;
				command->arguments[i + 1] = NULL;
				i++;
			}
		}
		else if (ft_strcmp(command->arguments[i], ">>") == 0)
		{
			if (i < command->argument_count - 1)
			{
				command->output_redirection = command->arguments[i + 1];
				command->append_output = 1;
				command->arguments[i] = NULL;
				command->arguments[i + 1] = NULL;
				i++;
			}
		}
		else if (ft_strcmp(command->arguments[i], "<<") == 0)
		{
			if (i < command->argument_count - 1)
			{
				command->input_redirection = command->arguments[i + 1];
				command->append_output = 1;
				command->arguments[i] = NULL;
				command->arguments[i + 1] = NULL;
				i++;
			}
		}
		else if (ft_strcmp(command->arguments[i], "|") == 0)
		{
			command->has_pipe = 1;
			command->pipe_command = command->arguments[i + 1];
			command->arguments[i] = NULL;
			command->arguments[i + 1] = NULL;
			i++;
		}
		else if (command->arguments[i][0] == '$')
		{
			env_variable = getenv(&command->arguments[i][1]);
			if (env_variable != NULL)
			{
				free(command->arguments[i]);
				command->arguments[i] = ft_strdup(env_variable);
			}
		}
		else if (ft_strcmp(command->arguments[i], "$?") == 0)
		{
			exit_status_str = ft_itoa(command->last_exit_status);
			if (exit_status_str != NULL)
			{
				free(command->arguments[i]);
				command->arguments[i] = ft_strdup(exit_status_str);
				free(exit_status_str);
			}
		}
		i++;
	}
}

void    clear_tokens(t_command *command)
{
    free(command->command_name);

    for (int i = 0; i < command->argument_count; i++)
    {
        free(command->arguments[i]);
    }
    command->argument_count = 0;
}

void    tokenize_command(const char *input, t_command *command)
{

    clear_tokens(command);

    char *token;
    char *input_copy = ft_strdup(input); // Copie pour éviter de modifier l'input d'origine
    if (!input_copy)
    {
        perror("Erreur d'allocation de mémoire");
        exit(EXIT_FAILURE);
    }

    int inside_quotes = 0; // Indicateur pour suivre si nous sommes à l'intérieur de guillemets
    char *saveptr = NULL;

    // Le premier token est généralement la commande elle-même
    token = ft_strtok_r(input_copy, " \t\n", &saveptr);
    if (token != NULL)
    {
        command->command_name = ft_strdup(token);
        if (!command->command_name)
        {
            perror("Erreur d'allocation de mémoire");
            exit(EXIT_FAILURE);
        }
        command->argument_count = 1;
    }

    while (token != NULL)
    {
        // Si nous sommes à l'intérieur de guillemets, concaténer les tokens en un argument
        if (inside_quotes)
        {
            size_t len = ft_strlen(command->arguments[command->argument_count - 1]);
            command->arguments[command->argument_count - 1] = ft_realloc(
                command->arguments[command->argument_count - 1], len + ft_strlen(token) + 2);
            if (!command->arguments[command->argument_count - 1])
            {
                perror("Erreur d'allocation de mémoire");
                exit(EXIT_FAILURE);
            }
            ft_strcat(command->arguments[command->argument_count - 1], " ");
            ft_strcat(command->arguments[command->argument_count - 1], token);
        }
        else
        {
            // Si ce token commence par un guillemet, nous sommes à l'intérieur de guillemets
            if (token[0] == '"' || token[0] == '\'')
            {
                inside_quotes = 1;
                // Supprimer le guillemet ouvrant du début du token
                ft_memmove(token, token + 1, ft_strlen(token));
                token[ft_strlen(token) - 1] = '\0';
            }

            command->arguments[command->argument_count] = ft_strdup(token);
            if (!command->arguments[command->argument_count])
            {
                perror("Erreur d'allocation de mémoire");
                exit(EXIT_FAILURE);
            }
            command->argument_count++;
        }

        // Vérifier si ce token se termine par un guillemet
        size_t len = ft_strlen(token);
        if (len > 0 && (token[len - 1] == '"' || token[len - 1] == '\''))
        {
            inside_quotes = 0;
            // Supprimer le guillemet de fin du token
            token[len - 1] = '\0';
        }
        token = ft_strtok_r(NULL, " \t\n", &saveptr);
    }
    free(input_copy); // Libérer la copie de l'input
}

char	*read_user_input()
{
	char	*input = readline("minishell> ");
	size_t	input_len;

	if (input == NULL)
	{
		printf("Erreur lors de la lecture de l'entree\n");
		exit(EXIT_FAILURE);
	}
	input_len = ft_strlen(input);
	if (input_len > 0 && input[input_len - 1] == '\n')
		input[input_len - 1] = '\0';
	if (input[0] != '\0')
		add_history(input);
	return (input);
}

void disable_canonical_mode()
{
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &new_termios);
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

int main() {
    char *input;
    t_command command;
    struct termios orig_termios;
    struct termios new_termios;

	disable_canonical_mode();
    tcgetattr(STDIN_FILENO, &orig_termios);
    new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    signal(SIGINT, ctrl_c_handler);
    signal(SIGQUIT, SIG_IGN);
	rl_bind_key('\t', rl_complete);
    while (1) {
        if (ctrl_c_pressed) {
            ctrl_c_pressed = 0;
            printf("\nminishell> ");
            fflush(stdout);
        }
        input = readline("minishell> ");
        if (!input) {
            printf("Au revoir !\n");
            break ;
        }
        if (ft_strlen(input) > 0) {
            add_history(input);
        }
        printf("Commande saisie : %s\n", input);

        if (ft_strcmp(input, "exit") == 0) {
            printf("Au revoir !\n");
            free(input);
            break;
        }
        if (est_builtin(input))
            execute_builtin(input, command.arguments, command.argument_count);
        else {
            tokenize_command(input, &command);
            analyze_tokens(&command);
            execute_command(&command);
        }
        free(input);
    }
    clear_history();
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    return 0;
}

