import React from 'react';
import { render } from 'react-dom'
import { Router, Route, IndexRoute, Link, hashHistory } from 'react-router'

import SkyPlannerApp from './skyplanner-app';
import SkyPlannerHomePage from './skyplanner-homepage'
import SkyPlannerLoginPage from './skyplanner-loginpage'
import { NotificationContainer } from 'react-notifications';

require('style!react-notifications/lib/notifications.css');

var getNavLinks = function() {
    return [{key: '/', display: 'Home'}, {key: '/login', display: 'Login'}];
}

render(
    <div>
        <Router history={hashHistory}>
            <Route path="/" component={SkyPlannerApp} navs={getNavLinks()}>
                <IndexRoute component={SkyPlannerHomePage} />
                <Route path="login" component={SkyPlannerLoginPage} />
            </Route>
        </Router>
        <NotificationContainer />
    </div>,
    document.getElementById('content')
);


