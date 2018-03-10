from flask_wtf import FlaskForm
from wtforms import StringField, PasswordField, BooleanField, SubmitField
from wtforms.validators import DataRequired

class LoginForm(FlaskForm):
    username = StringField('Username', validators=[DataRequired()])
    password = PasswordField('Password', validators=[DataRequired()])
    remember_me = BooleanField('Remember Me')
    submit = SubmitField('Sign In')

class MakiForm(FlaskForm):
    activity = StringField('Activity', validators=[DataRequired()])
    location = StringField('Where?', validators=[DataRequired()])
    submit = SubmitField('OK')