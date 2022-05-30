class View:
    def start(self):
        print("### Sistema offline ####\n")
        print("Faça login como mesário para iniciar a votação.")
        return self.menu()

    def menu(self):
        print("0 - Sair")
        print("1 - Para fazer login como mesario")
        return int(input("\nDigite a opção: "))

    def get_login_detail(self):
        print("\nDigite o numero de login")
        login_number = int(input())
        print("\nDigite a senha de login")
        login_password = int(input())
        return login_number, login_password
    
    def show_login_detail(self, result):
        if result == True:
            print()
            print("Mesário autenticado com sucesso!")
        else:
            print()
            print("Mesário não autenticado.")
            print("Por favor, reinicie a autenticação.")

    def start_voting_machine(self):
        print()
        print("### Sistema de votação iniciada ###")
        print("### Para terminar a votação, digite o número 0 para o titulo de eleitor ###")
        print()

    def finish_voting_machine(self):
        print("Você quer finalizar a votação?")
        print("0 - Continuar votação")
        print("1 - Finalizar votação")
        option = int(input("\nDigite a opção: "))
        if option == 0:
            print()
            return 0

        # User wants to finish voting system
        print()
        print("Para finalizar a votação, escolha a opção 1")
        print("Logo após, será necessário digitar o login e senha do mesário")
        print()
        return self.menu()
    
    def get_voter_registration_number(self):
        return int(input("Digite o número do título de eleitor: "))

    def show_candidate_chosen(self, candidate_chosen):
        print()
        print("{}\n{}"
        .format(candidate_chosen['name'], candidate_chosen['political_party']))
        option = self.get_voter_candidate_option()
        return option

    def get_voter_candidate_option(self):
        print("Digite VERDE para CONFIRMAR este voto")
        print("Digite LARANJA para REINICIAR este voto")
        print()
        return input().lower()

    def voter_error(self, error):
        if error == 2:
            print()
            print("Eleitor não pode votar.")
            print("Eleitor tem cadastro irregular no TRE.")
            print()
        elif error == 3:
            print()
            print("Número de título de eleitor não reconhecido.")
            print("Digite novamente.")
            print()
        elif error == 4:
            print()
            print("Eleitor já votou nessa eleição.")
            print()

    def voter_is_voting(self, candidate):
        print("{}: ".format(candidate))
        return int(input())

    def voter_is_finished(self):
        print("FIM")

    def end(self, result):
        if result == True:
            print()
            print("### Sistema de votação finalizado com sucesso ###")
        else:
            print()
            print("### Sistema de votação finalizado com erro ###")

    def show_election_results(self, running_candidate, election_results, valid_votes):
        print()
        print("### Resultados para candidato a {} ###".format(running_candidate))
        print()
        for candidate, votes in election_results.items():
            if ((candidate == 'nulo' and votes == 0)
                or (candidate != 'nulo' and votes == 0)):
                print("> {}, {} votos, {:.2%} dos votos"
                .format(candidate, votes, votes)
                )
            else:
                print("> {}, {} votos, {:.2%} dos votos válidos"
                .format(candidate, votes, valid_votes/votes)
                )

        sort_election_results = sorted(election_results.items(), key=lambda y: y[1], reverse=True)
        print()
        print("-----------------------------------")
        print("Candidato eleito {}".format(sort_election_results[0][0]))
        print("-----------------------------------")