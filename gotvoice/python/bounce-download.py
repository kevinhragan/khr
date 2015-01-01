#! /usr/bin/python
# download all the mail in a mailbox
import getpass, imaplib

M = imaplib.IMAP4("mail.gotvoice.com")
M.login("bounce", "oatmealcookie")
M.select()
typ, data = M.search(None, 'ALL')
for num in data[0].split():
   typ, data = M.fetch(num, '(RFC822)') 
   print '%s\n%s\n' % ("From download",data[0][1])
M.close()
M.logout()
