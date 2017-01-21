import React from 'react';
import { render } from 'react-dom'
import { Router, Route, IndexRoute, Link, hashHistory } from 'react-router'

import SkyPlannerApp from './skyplanner-app';
import SkyPlannerHomePage from './skyplanner-homepage'
import SkyPlannerLoginPage from './skyplanner-loginpage'
import { NotificationContainer, NotificationManager } from 'react-notifications';
import Ajax from './ajax';
import LoginDispatcher from './login-dispatcher';

require('style!react-notifications/lib/notifications.css');

var history = hashHistory;

class RoutesContainer extends React.Component {
    constructor(props) {
        super(props);
    }
    
    componentDidMount() {
        LoginDispatcher.register(this);

        var token = window.localStorage.getItem('user_token');
        if(token != null) {
            Ajax.fetch('/api/users/get?auth=' + token)
                .then(Ajax.decode_json({
                    success: (j) => { Object.assign(j, {token: token}); LoginDispatcher.setUser(j); } 
            }));
        }

    }

    componentWillUnmount() {
        LoginDispatcher.unregister(this);
    }

    render() {
        return (
            <Router history={history} ref='router'>
                <Route path="/" component={(props) => <SkyPlannerApp navs={this.navs()} location={props.location} >{props.children}</SkyPlannerApp> }>
                    <IndexRoute component={SkyPlannerHomePage} />} />
                    <Route path="login" component={SkyPlannerLoginPage} />
                    <Route path='logout' component='div' onEnter={() => LoginDispatcher.setUser(undefined) } />
                </Route>
            </Router>
        );
    }

    navs() {
        return {
            loggedIn: [{key: '/', display: 'Home'}, {key: '/logout', display: 'Logout'}],
            loggedOut: [{key: '/', display: 'Home'}, {key: '/login', display: 'Login'}]
        };
    }

    setUser(user) {
        if(user === undefined) {
            window.localStorage.removeItem('user_token');
            NotificationManager.success('User logged out correctly', 'Logout', 5000);
        }
        history.push('/');
    }
}


render(
    <div>
        <RoutesContainer />
        <NotificationContainer />
    </div>,
    document.getElementById('content')
);


