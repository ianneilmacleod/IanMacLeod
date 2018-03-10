import os
from flask import render_template, flash, redirect, request
from app import app
from app.forms import LoginForm, MakiForm
import geosoft.gxpy.grid as gxgrd
import geosoft.gxpy.grid_utility as gxgrdu

@app.route('/')
@app.route('/index')
def index():
    user = {'username': 'Ian MacLeod'}
    posts = [
        {
            'author': {'username': 'ian.macleod@geosoft.com'},
            'body': 'Beautiful day in Port Credit!'
        },
        {
            'author': {'username': 'Natalie'},
            'body': 'Working out!'
        }
    ]
    return render_template('index.html', title='Home', user=user, posts=posts)


@app.route('/login', methods=['GET', 'POST'])
def login():

    form = LoginForm()
    if form.validate_on_submit():
        message = 'Login requested for user {}, remember_me={}'.format(
            form.username.data, form.remember_me.data)
        flash(message)
        return redirect('/index')

    return render_template('login.html', title='Sign In', form=form)


@app.route('/maki', methods=['GET', 'POST'])
def maki():

    # get derivative argument as '?d'
    derivative = request.args.get('d', default=-1, type=int)
    if derivative < -1 or derivative > gxgrdu.TILT_ANGLE:
        derivative = gxgrdu.TILT_ANGLE
    cs = request.args.get('cs', default='')

    # get coordinate system

    # process the grid and create an image
    grid_file = 'bhn_tmi_250m.grd'
    grid_path = './app/data/{}'.format(grid_file)
    if derivative >= 0:
        image_ref = '/static/{}.d{}.png'.format(grid_file, str(derivative))
    elif cs:
        image_ref = '/static/{}.cs.png'.format(grid_file)
    else:
        image_ref = '/static/{}.png'.format(grid_file)
    image_path = './app' + image_ref
    if (not os.path.exists(image_path)) or os.path.getmtime(image_path) < os.path.getmtime(grid_path):
        grd = gxgrd.Grid.open(grid_path, mode=gxgrd.FILE_READ, coordinate_system=cs)
        if derivative == -1:
            grd.image_file(image_path, shade=True, pix_width=500)
        else:
            gxgrdu.derivative(grd, int(derivative)).image_file(image_path, shade=True, pix_width=500)

    return render_template('maki.html', title='Maki\'s page', image=image_ref)
