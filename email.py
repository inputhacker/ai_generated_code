import smtplib
from email.message import EmailMessage

smtp_server = 'smtp.example.com'
smtp_port = 587
smtp_username = 'your_username'
smtp_password = 'your_password'

msg = EmailMessage()
msg['From'] = 'sender@example.com'
msg['To'] = 'recipient1@example.com'
msg['Cc'] = 'ccperson@example.com'
msg['Subject'] = 'Test Subject'

msg.set_content('This is the body of the test email sent from Python.')

recipients = [msg['To']] + [msg['Cc']]  # 참조자와 함께

with smtplib.SMTP(smtp_server, smtp_port) as server:
    server.starttls()
    server.login(smtp_username, smtp_password)
    server.send_message(msg, from_addr=msg['From'], to_addrs=recipients)
