import React from 'react';
import { render } from 'react-dom'
import { Router, Route, IndexRoute, Link, hashHistory } from 'react-router'

import SkyPlannerApp from './skyplanner-app';
import SkyPlannerHomePage from './skyplanner-homepage'
import SkyPlannerLoginPage from './skyplanner-loginpage'
import { NotificationContainer, NotificationManager } from 'react-notifications';
import Ajax from './ajax';
import AuthManager from './auth-manager';

require('style!react-notifications/lib/notifications.css');

var history = hashHistory;

class RoutesContainer extends React.Component {
    constructor(props) {
        super(props);
    }
    
    componentDidMount() {
        AuthManager.register(this);

        var token = window.localStorage.getItem('user_token');
        if(token != null) {
            Ajax.fetch('/api/users/get?auth=' + token)
                .then(Ajax.decode_json({
                    success: (j) => { Object.assign(j, {token: token}); AuthManager.setUser(j); } 
            }));
        }

    }

    componentWillUnmount() {
        AuthManager.unregister(this);
    }

    render() {
        return (
            <Router history={history} ref='router'>
                <Route path="/" component={SkyPlannerApp} navs={this.navs()}>
                    <IndexRoute component={SkyPlannerHomePage} />} />
                    <Route path="login" component={SkyPlannerLoginPage} />
                    <Route path='logout' component='div' onEnter={() => AuthManager.setUser(null) } />
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
        if(user == null) {
            window.localStorage.removeItem('user_token');
            NotificationManager.success('User logged out correctly', 'Logout', 5000);
        }
        history.push('/');
    }
}


var requireAuth = function() {
    if (!AuthManager.user() ) {
        replace({
            pathname: '/login',
            state: { nextPathname: nextState.location.pathname }
        })
    }
}


render(
    <div>
        <RoutesContainer />
        <NotificationContainer />
    </div>,
    document.getElementById('content')
);


