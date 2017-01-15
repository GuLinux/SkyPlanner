class Hello extends React.Component {
    render() {
        return <h1>Hello, {this.props.name}</h1>;
    }
}

ReactDOM.render(
  <Hello name="Marco" />,
  document.getElementById('content')
);
