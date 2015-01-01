#! /usr/bin/python
# erase all the mail from admin@heket
import getpass, imaplib

M = imaplib.IMAP4("mail.gotvoice.com")
M.login("admin", getpass.getpass())
M.select()
typ, data = M.search(None, '(LARGER 50000)')
for num in data[0].split():
    M.store(num, '+FLAGS.SILENT', '\\Deleted')
M.expunge()
M.logout()
