import React from 'react'
import ReactDOM from 'react-dom'
import { Button } from 'react-toolbox/lib/button';

class Hello extends React.Component {
    render() {
        return <h1>Hello</h1>;
    }
};

ReactDOM.render(
    <Button label="Hello World!" />,
    document.getElementById('content')
);
