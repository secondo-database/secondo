First you have to install postfix to use the sendmail operator.
On Suse 42.2 is installed by default. You can use Yast if this is not the case. 

Use "sudo apt install postfix" to install it on Ubuntu.


You have to make sure that the bash is your standart shell.
On Ubuntu you have to change the standart from dash to bash.

Use the command :

sudo dpkg-reconfigure dash 

and then choose the "No" Option.








Then you have to modify two files 
/etc/postfix/main.cf  and /etc/postfix/sasl_passwd.

If the later one does not exists, you have to create it.





In the sasl_passwd.db you have to edit (default port 25):

[smtp.isp.example]:25  username:password



In the main.cf you have to adapt the entries to your purposes (smtp Server without tls).

Either you have to change existing entries or you have to create them.


#specify SMTP relay host 
relayhost = [smtp.isp.example]:25 

# enable SASL authentication 
smtp_sasl_auth_enable = yes

# disallow methods that allow anonymous authentication. 
smtp_sasl_security_options = noanonymous

# where to find sasl_passwd
smtp_sasl_password_maps = hash:/etc/postfix/sasl_passwd

# Enable STARTTLS encryption 
smtp_use_tls = no









Example for a smtp server with tls:


In the sasl_passwd.db you have to edit:

[smtp.isp.example]:587  username:password





In the main.cf you have to edit:

# specify SMTP relay host 
relayhost = [smtp.isp.example]:587 


# enable SASL authentication 
smtp_sasl_auth_enable = yes

# disallow methods that allow anonymous authentication. 
smtp_sasl_security_options = noanonymous

# where to find sasl_passwd
smtp_sasl_password_maps = hash:/etc/postfix/sasl_passwd

# Enable STARTTLS encryption 
smtp_use_tls = yes

# where to find CA certificates
smtp_tls_CAfile = /etc/ssl/certs/ca-certificates.crt








As a last step you have to restart the postfix service:

sudo service postfix restart
