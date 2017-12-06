import string
import random
import sys

def geraArquivo (tamanho):
	for i in range (tamanho):
		print(random.choice(string.letters))


geraArquivo(31457280)

