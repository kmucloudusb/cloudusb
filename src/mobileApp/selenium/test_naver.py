#coding: utf-8

from selenium import webdriver
from bs4 import BeautifulSoup


#네이버

driver = webdriver.Chrome('/Users/jinheesang/chromedriver')
driver.implicitly_wait(3)
driver.get('https://nid.naver.com/nidlogin.login')
driver.find_element_by_name('id').send_keys('jinheesang')
driver.find_element_by_name('pw').send_keys('ab1221816')
driver.find_element_by_xpath('//*[@id="frmNIDLogin"]/fieldset/input').click()

#구글
"""
driver = webdriver.Chrome('/Users/jinheesang/chromedriver')
driver.implicitly_wait(3)
driver.get('https://accounts.google.com/')

driver.find_element_by_name('identifier').send_keys('jinheesang@gmail.com')
driver.find_element_by_id('identifierNext').click()

driver.find_element_by_name('password').send_keys('00aa8410')
"""