<!DOCTYPE html>


<html>

    <head>

        <title>TamataSpiru</title>

        <style>

            a {text-decoration: none; color: black;}

        </style>

    </head>


    <body>

        <h1>Sensors Capture in realTime : </h1>


        <ul>

        <% todolist.forEach(function(todo, index) { %>

            <li><a href="/todo/supprimer/<%= index %>">✘</a> <%= todo %></li>

        <% }); %>

        </ul>


        <form action="/todo/ajouter/" method="post">

            <p>

                <label for="newtodo">Que dois-je faire ?</label>

                <input type="text" name="newtodo" id="newtodo" autofocus />

                <input type="submit" />

            </p>

        </form>

    </body>

</html>
