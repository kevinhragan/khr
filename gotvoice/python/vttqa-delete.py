#! /usr/bin/python
# erase old mail from admin@heket
import getpass, imaplib, time

#string rep of date ten days ago
limit = time.strftime("%d-%b-%Y",time.gmtime(time.time()-(10*24*60*60)))

M = imaplib.IMAP4("mail.gotvoice.com")
M.login("vttqa", getpass.getpass())
M.select()
#typ, data = M.search(None, '(BEFORE limit)')
typ, data = M.search(None, '(BEFORE ' + limit + ')')
for num in data[0].split():
    M.store(num, '+FLAGS.SILENT', '\\Deleted')
M.expunge()
M.logout()
