<?php

$options = new \Couchbase\ClusterOptions();
$options->credentials('Administrator', 'password');
$cluster = new \Couchbase\Cluster('couchbase://localhost', $options);

$roles = $cluster->users()->getRoles();
printf("There are %d roles on the cluster\n", count($roles));
$first_role = $roles[0];
printf("First role is \"%s\":\n%s\n", $first_role->role()->name(),
        preg_replace('/(^|\n)/', "$1\t", wordwrap($first_role->description())));

$group_name = 'sample-managers';

$groups = $cluster->users()->getAllGroups();
printf("There are %d group(s) on the cluster:\n", count($groups));
$has_group = false;
foreach ($groups as $group) {
        printf("  * %s (%d roles)\n", $group->name(), count($group->roles()));
        if ($group->name() == $group_name) {
                $has_group = true;
        }
}

if ($has_group) {
        $start = microtime(true);
        $cluster->users()->dropGroup($group_name);
        printf("Group \"%s\" has been removed in %fus\n", $group_name, microtime(true)-$start);
}

$group = new \Couchbase\Group();
$group->setName($group_name);
$group->setDescription('Users who have full access to sample buckets');
$group->setRoles([
        (new \Couchbase\Role())->setName('bucket_admin')->setBucket('travel-sample'),
        (new \Couchbase\Role())->setName('bucket_full_access')->setBucket('travel-sample'),
        (new \Couchbase\Role())->setName('bucket_admin')->setBucket('beer-sample'),
        (new \Couchbase\Role())->setName('bucket_full_access')->setBucket('beer-sample'),
]);

$start = microtime(true);
$cluster->users()->upsertGroup($group);
printf("Group \"%s\" has been created in %fus\n", $group->name(), microtime(true)-$start);

$group = $cluster->users()->getGroup($group_name);
printf("Group \"%s\" (%s)\n  Roles:\n", $group->name(), $group->description());
foreach ($group->roles() as $role) {
        printf("    * %s", $role->name());
        if ($role->bucket() != null) {
                printf("[%s]", $role->bucket());
        }
        printf("\n");
}

$user_name = 'guest';
$has_user = false;

$users = $cluster->users()->getAllUsers();
printf("There are %d users on the cluster\n", count($users));
function dumpUserMeta($meta) {
    $user = $meta->user();
    if ($user->username() == $user_name) {
        $has_user = true;
    }
    printf("%s (%s), domain: %s\n", $user->username(), $user->displayName(), $meta->domain());
    printf("    the password has been changed at %s\n", $meta->passwordChanged());
    printf("    member of %d group(s)\n", count($user->groups()));
    foreach ($user->groups() as $group) {
        printf("        * %s\n", $group);
    }
    printf("    member of %d external group(s)\n", count($meta->externalGroups()));
    foreach ($meta->externalGroups() as $group) {
        printf("        * %s\n", $group);
    }
    printf("    has %d role(s) assigned directly\n", count($user->roles()));
    foreach ($user->roles() as $role) {
        printf("        * %s", $role->name());
        if ($role->bucket() != null) {
            printf("[%s]", $role->bucket());
        }
        printf("\n");
    }
    printf("    has %d effective roles:\n", count($meta->effectiveRoles()));
    $roles_by_origin = [
        'user' => [],
        'group' => []
    ];
    foreach ($meta->effectiveRoles() as $role) {
        $origin_info = "";
        foreach ($role->origins() as $origin) {
            if ($origin->type() == "group") {
                if (isset($roles_by_origin['group'][$origin->name()])) {
                    array_push($roles_by_origin['group'][$origin->name()], $role->role());
                } else {
                    $roles_by_origin['group'][$origin->name()] = [$role->role()];
                }
            } else {
                array_push($roles_by_origin['user'], $role->role());
            }
        }
    }
    foreach ($roles_by_origin['user'] as $role) {
        printf("        * %s", $role->name());
        if ($role->bucket() != null) {
            printf("[%s]", $role->bucket());
        }
        printf("%s\n", $origin_info);
    }
    foreach (array_keys($roles_by_origin['group']) as $group) {
        printf("        inherited from \"%s\":\n", $group);
        foreach ($roles_by_origin['group'][$group] as $role) {
            printf("            * %s", $role->name());
            if ($role->bucket() != null) {
                printf("[%s]", $role->bucket());
            }
            printf("%s\n", $origin_info);
        }
    }
}

for ($idx = 0; $idx < count($users); $idx++) {
    printf("%d. ", $idx);
    dumpUserMeta($users[$idx]);
}

if ($has_user) {
        $start = microtime(true);
    $cluster->users()->dropUser($user_name);
        printf("User \"%s\" has been removed in %fus\n", $user_name, microtime(true)-$start);
}

$role = new \Couchbase\Role();
$role->setName('bucket_full_access');
$role->setBucket('*');

$user = new \Couchbase\User();
$user->setUsername($user_name);
$user->setPassword('secret');
$user->setDisplayName('Guest User');
$user->setGroups(['sample-managers']);
$user->setRoles([$role]);
$start = microtime(true);
$cluster->users()->upsertUser($user);
printf("User \"%s\" has been created in %fus\n", $user_name, microtime(true)-$start);

$user_meta = $cluster->users()->getUser($user_name);
dumpUserMeta($user_meta);
