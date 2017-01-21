import React from 'react';
import { render } from 'react-dom'
import { Router, Route, IndexRoute, Link, hashHistory } from 'react-router'

import SkyPlannerApp from './skyplanner-app';
import SkyPlannerHomePage from './skyplanner-homepage'
import SkyPlannerLoginPage from './skyplanner-loginpage'
import { NotificationContainer, NotificationManager } from 'react-notifications';
import Ajax from './ajax';
import AuthManager from './auth-manager';
import URLs from './urls';
require('style!react-notifications/lib/notifications.css');

var history = hashHistory;
class RoutesContainer extends React.Component {
    constructor(props) {
        super(props);
    }
    
    componentDidMount() {
        AuthManager.register(this);

        let token = AuthManager.token();
        if(token != null) {
            Ajax.fetch( URLs.buildAuthPath('/api/users/get'))
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
                <Route path={URLs.root.route} component={SkyPlannerApp} navs={this.navs()}>
                    <IndexRoute component={SkyPlannerHomePage} />} />
                    <Route path={URLs.login.route} component={SkyPlannerLoginPage} />
                    <Route path={URLs.logout.route} component='div' onEnter={() => AuthManager.setUser(null) } />
                </Route>
            </Router>
        );
    }

    navs() {
        return {
            loggedIn: [{key: URLs.root.path, display: 'Home'}, {key: URLs.logout.path, display: 'Logout'}],
            loggedOut: [{key: URLs.root.path, display: 'Home'}, {key: URLs.login.path, display: 'Login'}]
        };
    }

    setUser(user) {
        if(user == null) {
            window.localStorage.removeItem('user_token');
            NotificationManager.success('User logged out correctly', 'Logout', 5000);
        }
        history.replace('/');
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


