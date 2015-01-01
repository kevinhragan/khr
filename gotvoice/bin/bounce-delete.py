#! /usr/bin/python
# erase all the mail in a mailbox
import getpass, imaplib

M = imaplib.IMAP4("mail.gotvoice.com")
M.login("bounce", getpass.getpass())
M.select()
typ, data = M.search(None, 'ALL')
for num in data[0].split():
    M.store(num, '+FLAGS.SILENT', '\\Deleted')
M.expunge()
M.logout()
