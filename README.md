# prinit
Prinit is a tool made for windows to start your projects faster.
Here is the version for linux users.

# command

>**If you want to use prinit you'll probably want to add it to the _path_**

- Download last release of prinit
- Copy prinit folder and paste it in your home folder (You can add a '.' before it's name to make it hidden)
- Add "~/prinit" or "~/.prinit" folder to $PATH : https://linuxize.com/post/how-to-add-directory-to-path-in-linux/

You can use prinit like this :

`prinit [project_name] [project_workspace] [project_type]`

Prinit will create a folder called *project_name* at folder path *project_workspace* with files of template folder *project_type*.

If you do :

`prinit my_project ~/home/[username]/ cpp`

You will have a folder called my_project in your $HOME with the basic files for a cpp project.

# settings

With *setting.txt* you can change the command order. `[project_name] [project_workspace] [project_type]`

With *workspaces.txt* you can create a shortcut for a folder path that you use frequently.

# workspaces

Add this line in *workspaces.txt* :

`Desktop : ~/home/[your_username]`

And simply run the command :

`prinit my_project Desktop cpp`

And you will have the same result as before!

_**There is a template folder with subfolders, these subfolders are the project_types, you can modify them as you please and/or add new ones.**_

# templates

_templates_ is the folder where templates folders are stored (cpp, c, etc...).

- You can rename these folders as you please
- You can modify their content (add files, modify files, etc...)
- You can create new folders

Everything will be handled automatically !
