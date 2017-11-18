import requests
from lxml import html
from bs4 import BeautifulSoup

google = requests.get('https://google.com')
soup = BeautifulSoup(google.content, 'html.parser')

print(soup.title.string)
#print(soup.title)
#print(soup.find_all('a'))
for link in soup.find_all('a'):
    print(link.get('href'))

