import React from 'react';
import { render } from 'react-dom'
import { Router, Route, IndexRoute, Link, hashHistory } from 'react-router'

import SkyPlannerApp from './skyplanner-app';
import SkyPlannerHomePage from './skyplanner-homepage'
import SkyPlannerLoginPage from './skyplanner-loginpage'
import { NotificationContainer } from 'react-notifications';

require('style!react-notifications/lib/notifications.css');


class Entrypoint extends React.Component{
    constructor(props) {
        super(props);
        this.state = {user: undefined};
    }
    render() {
        return (
            <Router history={hashHistory}>
                <Route path="/" component={SkyPlannerApp} navs={this.getNavLinks()}>
                    <IndexRoute component={SkyPlannerHomePage} />
                    <Route path="login" component={SkyPlannerLoginPage} onLogin={this.setUser.bind(this)} />
                </Route>
            </Router>
        )
    }

    setUser(user) {
        this.setState({user: user});
    }

    getNavLinks() {
        if(this.state.user === undefined)
            return [{key: '/', display: 'Home'}, {key: '/login', display: 'Login'}];
        return [{key: '/', display: 'Home'}, {key: '/logout', display: 'Logout'}];
    }
}

render(
    <div>
        <Entrypoint />
        <NotificationContainer />
    </div>,
    document.getElementById('content')
);


