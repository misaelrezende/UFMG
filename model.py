from candidate import Candidate
from voter import Voter
from clerk import Clerk

# results file header
# candidate_name, # of votes received
# null votes,     # of votes received

# candidates file header
# candidate_name, number, name of party

class Model:

    # Return election_results, valid_votes
    def get_election_results(self, candidate):
        election_results = {}

        # open results file
        results_file = "files/results_{}.txt".format(candidate)
        with open(results_file, 'r') as results:
            line = results.readline()
            candidate_name, votes = line.split(',')
            election_results[candidate_name] = votes

        return election_results

    # Return if user is able to login the voting system
    def login(self, login_number, login_password):
        is_user_allowed = False
        with open("users_login.txt", 'r') as reader:
            line = reader.readline()
            while line != '':
                user, password = line.split(',')
                if int(login_number) == int(user):
                    if int(password) == int(login_password):
                        is_user_allowed = True
                line = reader.readline()

        return is_user_allowed

    # Return name and political party of candidate
    # NOTE Assume input is correct and candidate exists
    def get_candidate_info(self, candidate_chosen):
        candidate_info = {}
        with open("files/candidates_{}.txt".format(candidate_chosen)) as reader:
            line = reader.readline()
            while line != '':
                candidate_name,number,party = line.split(',')
                if int(number) == candidate_chosen:
                    candidate_info['name'] = candidate_name
                    candidate_info['political_party'] = party

        return candidate_info

    def verify_voter():
        # 1: eleitor está apto a votar;
        # 2: não pode votar;
        # 3: núm incorreto;
        # 4: já votou


    def compute_vote(candidate_chosen):
        pass